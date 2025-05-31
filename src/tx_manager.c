#include "tx_manager.h"
#include "utils.h"
#include "transaction.h"

#include <pthread.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <memory.h>

static int transaction_id;
static pthread_mutex_t tx_lock;

// todo: add tx_list

void kv_txm_init ()
{
    pthread_mutex_init (&tx_lock, NULL);
    
    // todo: get last tx id from recovery log
    transaction_id = 0;
}

void kv_txm_destroy ()
{
    pthread_mutex_destroy (&tx_lock);
}

struct kv_tx* kv_txm_start_new_transaction ()
{
    struct kv_tx *new_tx;
    int new_tx_id;
    
    pthread_mutex_lock (&tx_lock);
    new_tx_id = ++transaction_id;
    pthread_mutex_unlock (&tx_lock);

    new_tx = kv_tx_create (new_tx_id);

    return new_tx;
}

int kv_txm_end_transaction (struct kv_lm *lm, struct kv_tx *tx)
{
    kv_tx_unlock_all (lm, tx);
    kv_tx_destroy (tx);

    return 0;
}

struct kv_ll* kv_txm_ongoing_transactions ()
{
    // todo
    return NULL;
}
