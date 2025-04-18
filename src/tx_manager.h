#ifndef __KV_TX_MANAGER_H__
#define __KV_TX_MANAGER_H__

#include <stdint.h>

#include "linked_list.h"

void kv_txm_init ();
void kv_txm_init2 ();

struct kv_tx* kv_txm_start_new_transaction ();
int kv_txm_end_transaction (struct kv_tx *tx);

int kv_txm_add_end_log (int tx_id);

struct kv_ll* kv_txm_ongoing_transactions ();

#endif
