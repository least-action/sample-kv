#ifndef __KV_TRANSACTION_H__
#define __KV_TRANSACTION_H__

#include <stdint.h>

#include "linked_list.h"

void kv_tx_init ();
void kv_tx_init2 ();

int kv_tx_start_new_transaction ();

void kv_tx_end_transaction (int tx_id);

struct kv_ll* kv_tx_ongoing_transactions ();

#endif
