#include "transaction.h"
#include "lock_rwlock.h"
#include "kv_recovery.h"
#include "kv_command.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

struct kv_tx_lock_elem {
    char *key;
    size_t key_len;
    struct kv_tx_lock_elem *prev;
    struct kv_tx_lock_elem *next;
};

struct kv_tx {
    uint32_t id;
    struct kv_tx_lock_elem *lock_head;
    struct kv_tx_lock_elem *lock_tail;
    uint32_t last_lsn;
};

struct kv_tx* kv_tx_create (int tx_id)
{
    struct kv_tx *tx;
    tx = (struct kv_tx *) malloc (sizeof (struct kv_tx));
    tx->id = tx_id;
    tx->last_lsn = 0;
    tx->lock_head = NULL;
    tx->lock_tail = NULL;

    return tx;
}

int kv_tx_destroy (struct kv_tx *tx)
{
    // todo: check lock_head empty
    free (tx);
    return 0;
}

uint32_t kv_tx_get_id (struct kv_tx *tx)
{
    return tx->id;
}

uint32_t kv_tx_last_lsn (struct kv_tx *tx)
{
    return tx->last_lsn;
}

int kv_tx_set_last_lsn (struct kv_tx *tx, uint32_t new_lsn)
{
    tx->last_lsn = new_lsn;
    return 0;
}

int kv_tx_rollback (struct kv_tx *tx, struct kv_ht *ht, struct kv_lm *lm)
{
    uint32_t new_lsn;
    uint32_t next_crl_lsn;
    struct kv_recovery_log_line *log_line;
    struct kv_ht_kv del_data;
    struct key_data *del_key_data;
    struct val_data *del_val_data;

    char *key = NULL;
    char *value = NULL;
    struct key_data *k_data = NULL;
    struct val_data *v_data = NULL;
    struct val_data *prev_v_data = NULL;

    next_crl_lsn = tx->last_lsn;
    while (1) {
        log_line = kv_recovery_get_log (next_crl_lsn);
        if (log_line == NULL) {
            // todo: error handling
            perror ("log_line NULL");
            exit (1);
        }

        if (log_line->log_type == KV_REC_BEGIN) {
            kv_recovery_destroy_log_line (log_line);
            break;
        }

        if (log_line->log_type != KV_REC_UPDATE) {
            next_crl_lsn = log_line->prev_lsn;
            kv_recovery_destroy_log_line (log_line);
            continue;
        }

        // if log_type is UPDATE
        if (log_line->old_len != 0) {
            // restore value
            key = (char *) malloc (log_line->key_len);
            memcpy (key, log_line->key, log_line->key_len);
            value = (char *) malloc (log_line->old_len);
            memcpy (value, log_line->old_val, log_line->old_len);

            // todo: bug: key data malloc free when updated
            k_data = (struct key_data *) malloc (sizeof (struct key_data));
            k_data->key = key;
            k_data->key_len = log_line->key_len;
            v_data = (struct val_data *) malloc (sizeof (struct val_data));
            v_data->value = value;
            v_data->val_len = log_line->old_len;

            // no need to key lock
            new_lsn = kv_recovery_clr_log (
                kv_tx_last_lsn (tx), kv_tx_get_id (tx), log_line->prev_lsn,
                log_line->key, log_line->key_len, log_line->new_val, log_line->new_len, log_line->old_val, log_line->old_len
            );
            kv_tx_set_last_lsn (tx, new_lsn);

            prev_v_data = kv_ht_set (ht, k_data, v_data);
            if (prev_v_data != NULL) {
                free (prev_v_data->value);
                free (prev_v_data);
            }
        } else {
            // delete value
            if (log_line->new_len == 0) {
                new_lsn = kv_recovery_clr_log (
                    kv_tx_last_lsn (tx), kv_tx_get_id (tx), log_line->prev_lsn,
                    log_line->key, log_line->key_len, NULL, 0, NULL, 0
                );
                kv_tx_set_last_lsn (tx, new_lsn);
            } else {
                key = (char *) malloc (log_line->key_len);
                memcpy (key, log_line->key, log_line->key_len);

                // todo: bug: key data malloc free when updated
                k_data = (struct key_data *) malloc (sizeof (struct key_data));
                k_data->key = key;
                k_data->key_len = log_line->key_len;
                v_data = (struct val_data *) malloc (sizeof (struct val_data));
                v_data->value = value;
                v_data->val_len = log_line->old_len;

                new_lsn = kv_recovery_clr_log (
                    kv_tx_last_lsn (tx), kv_tx_get_id (tx), log_line->prev_lsn,
                    log_line->key, log_line->key_len, log_line->new_val, log_line->new_len, NULL, 0
                );
                kv_tx_set_last_lsn (tx, new_lsn);

                del_data = kv_ht_del (ht, k_data);

                del_key_data = del_data.key;  // todo: check
                del_val_data = del_data.value;
                if (del_key_data != NULL && del_val_data != NULL) {
                    free (del_key_data->key);
                    free (del_key_data);
                    free (del_val_data->value);
                    free (del_val_data);
                }
                free (key);
                free (k_data);
            }
        }
        next_crl_lsn = log_line->prev_lsn;
        kv_recovery_destroy_log_line (log_line);
    }

    // add end log
    new_lsn = kv_recovery_end_log (tx->last_lsn, tx->id);
    tx->last_lsn = new_lsn;
    
    // unlock rwlock

    return 0;
}

int kv_tx_add_lock (struct kv_tx *tx, char *key, size_t key_len, bool is_rlock)
{
    struct kv_tx_lock_elem *new_elem;

    new_elem = (struct kv_tx_lock_elem *) malloc (sizeof (struct kv_tx_lock_elem));
    new_elem->key = malloc (key_len);
    memcpy (new_elem->key, key, key_len);
    new_elem->key_len = key_len;
    new_elem->next = NULL;
    new_elem->prev = tx->lock_tail;

    if (tx->lock_head == NULL) {    
        tx->lock_head = new_elem;
        tx->lock_tail = new_elem;
    } else {
        tx->lock_tail->next = new_elem;
        tx->lock_tail = new_elem;
    }
    return 0;
}

int kv_tx_unlock_all (struct kv_lm *lm, struct kv_tx *tx)
{
    struct kv_tx_lock_elem *del_elem;

    while (tx->lock_tail != NULL) {
        del_elem = tx->lock_tail;
        tx->lock_tail = tx->lock_tail->prev;
        if (tx->lock_tail != NULL)
            tx->lock_tail->next = NULL;  // todo: remove?
        kv_lm_unlock (lm, del_elem->key, del_elem->key_len);
        free (del_elem->key);
        free (del_elem);
    }
    return 0;
}
