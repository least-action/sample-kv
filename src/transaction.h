#ifndef __KV_TRANSACTION_H__
#define __KV_TRANSACTION_H__

#include <stdint.h>

void kv_tx_init (uint32_t initial_id);

uint32_t kv_tx_get_new_transaction ();

#endif
