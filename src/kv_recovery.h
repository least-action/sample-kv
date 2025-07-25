#ifndef __KV_RECOVERY_H__
#define __KV_RECOVERY_H__

#include "tx_manager.h"
#include "kv_hash.h"

#include <stdint.h>
#include <stddef.h>

#define KV_RECOVERY_MAX_LINE_LEN 128  // todo: check if longer


typedef uint32_t lsn_id;

enum kv_recovery_log_type {
    KV_REC_BEGIN,
    KV_REC_COMMIT,
    KV_REC_UPDATE,
    KV_REC_ABORT,
    KV_REC_CLR,
    KV_REC_CHECK,
    KV_REC_END,
};

struct kv_recovery_tx {
    uint32_t tx_id;
    uint32_t last_lsn;
};

struct kv_recovery_log_line {
    uint32_t lsn;
    uint32_t prev_lsn;
    uint32_t tx_id;
    enum kv_recovery_log_type log_type;
    uint32_t undo_next_lsn;
    char *key;
    size_t key_len;
    char *old_val;
    size_t old_len;
    char *new_val;
    size_t new_len;
    size_t tx_count;
    struct kv_recovery_tx *tx_list;
};

int kv_recovery_lock ();
int kv_recovery_unlock ();

int kv_recovery_recover ();
int kv_recovery_redo (struct kv_ht *ht);

int kv_recovery_init ();
int kv_recovery_destroy ();

uint32_t kv_recovery_begin_log (lsn_id prev_id, uint32_t tx_id);
uint32_t kv_recovery_commit_log (lsn_id prev_id, uint32_t tx_id);
uint32_t kv_recovery_update_log (lsn_id prev_id, uint32_t tx_id, char *key, size_t key_len, char *old_val, size_t old_len, char *new_val, size_t new_len);
uint32_t kv_recovery_abort_log (lsn_id prev_id, uint32_t tx_id);
uint32_t kv_recovery_clr_log (lsn_id prev_id, uint32_t tx_id, uint32_t undo_next_lsn, char *key, size_t key_len, char *cur_val, size_t cur_len, char *prev_val, size_t prev_len);
uint32_t kv_recovery_check_log (struct kv_ongoing_tx *tx_list, size_t list_size);
uint32_t kv_recovery_end_log (lsn_id prev_id, uint32_t tx_id);

struct kv_recovery_log_line* kv_recovery_get_log (lsn_id lsn);
int kv_recovery_destroy_log_line (struct kv_recovery_log_line *log);
 
#endif
