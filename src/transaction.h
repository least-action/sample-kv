#ifndef __KV_TRANSACTION_H__
#define __KV_TRANSACTION_H__

#include <stdint.h>

struct kv_tx_id {
    int tx_id;
    struct kv_tx_id *next;
    struct kv_tx_id *prev;
};

void kv_tx_init ();

int kv_tx_start_new_transaction ();

void kv_tx_end_transaction (int tx_id);

struct kv_tx_id* kv_tx_ongoing_transactions ();

#endif
