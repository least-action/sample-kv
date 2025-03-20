#ifndef __KV_TRANSACTION_H__
#define __KV_TRANSACTION_H__

#include <stdint.h>

void kv_tx_init (int initial_id);

int kv_tx_get_new_transaction ();

#endif
