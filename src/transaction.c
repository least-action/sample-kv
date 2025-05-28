#include "transaction.h"
#include "lock_rwlock.h"
#include "kv_recovery.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

struct lock_elem {
    struct kv_rwl *rwl;
    struct lock_elem *next;
};

struct kv_tx {
    uint32_t id;
    struct lock_elem *head;
    uint32_t last_lsn;
};

struct kv_tx* kv_tx_create (int tx_id)
{
    struct kv_tx *tx;
    tx = (struct kv_tx *) malloc (sizeof (struct kv_tx));
    tx->id = tx_id;
    tx->last_lsn = 0;

    return tx;
}

int kv_tx_destroy (struct kv_tx *tx)
{
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

int kv_tx_rollback (struct kv_tx *tx)
{
    uint32_t new_lsn;
    uint32_t next_crl_lsn;
    struct kv_recovery_log_line *log_line;

    // process undo
    next_crl_lsn = tx->last_lsn;
    while (1) {
        log_line = kv_recovery_get_log (next_crl_lsn);
        if (log_line == NULL) {
            // todo: error handling
            perror ("log_line NULL");
            exit (1);
        }

        if (log_line->log_type == KV_REC_UPDATE) {
            new_lsn = kv_recovery_clr_log (
                kv_tx_last_lsn (tx), kv_tx_get_id (tx), log_line->prev_lsn,
                log_line->key, log_line->key_len, log_line->new_val, log_line->new_len, log_line->old_val, log_line->old_len
            );
            kv_tx_set_last_lsn (tx, new_lsn);
            next_crl_lsn = log_line->prev_lsn;
        } else if (log_line->log_type == KV_REC_BEGIN) {
            kv_recovery_destroy_log_line (log_line);
            break;
        } else {
            // pass
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