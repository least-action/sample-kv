#ifndef __KV_TX_MANAGER_H__
#define __KV_TX_MANAGER_H__

#include "lock_manager.h"
#include "linked_list.h"

#include <stdint.h>
#include <pthread.h>

#define KV_TX_ID_FILE "tx_id.kvdb"

struct kv_ongoing_tx {
    uint32_t tx_id;
    uint32_t last_lsn;
};

void kv_txm_init ();
void kv_txm_destroy ();

int kv_txm_lock ();
int kv_txm_unlock ();
struct kv_tx* kv_txm_start_new_transaction ();
int kv_txm_end_transaction (struct kv_lm *lm, struct kv_tx *tx);

// return array size
size_t kv_txm_ongoing_transaction_ids (struct kv_ongoing_tx **array_start);

#endif
