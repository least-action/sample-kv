#ifndef __KV_TRANSACTION_H__
#define __KV_TRANSACTION_H__

#include "lock_rwlock.h"
#include "kv_hash.h"
#include "lock_manager.h"

#include <stdint.h>
#include <stdbool.h>

#define TX_ID_HEX_LEN 8  // todo: apply

struct kv_tx;

struct kv_tx* kv_tx_create (int tx_id);
int kv_tx_destroy (struct kv_tx *tx);

uint32_t kv_tx_get_id (struct kv_tx *tx);
uint32_t kv_tx_last_lsn (struct kv_tx *tx);
int kv_tx_set_last_lsn (struct kv_tx *tx, uint32_t new_lsn);
int kv_tx_rollback (struct kv_tx *tx, struct kv_ht *ht, struct kv_lm *lm);

int kv_tx_add_lock (struct kv_tx *tx, char *key, size_t key_len, bool is_rlock);
int kv_tx_unlock_all (struct kv_lm *lm, struct kv_tx *tx);

#endif
