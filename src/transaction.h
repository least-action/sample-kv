#ifndef __KV_TRANSACTION_H__
#define __KV_TRANSACTION_H__

#include "lock_rwlock.h"

struct kv_tx;

struct kv_tx* kv_tx_create (int tx_id);
int kv_tx_destroy (struct kv_tx *tx);

int kv_tx_get_id (struct kv_tx *tx);


#endif
