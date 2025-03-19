#include "transaction.h"
#include <pthread.h>

static uint32_t transaction_id;
static pthread_mutex_t tx_lock;


void kv_tx_init (uint32_t initial_id)
{
    if (initial_id < 0)
        initial_id = 0;
    transaction_id = initial_id;
    pthread_mutex_init (&tx_lock, NULL);
}

void kv_tx_destroy ()
{
    pthread_mutex_destroy (&tx_lock);
}

uint32_t kv_tx_get_new_transaction ()
{
    uint32_t new_tx_id;

    // pthread_mutex_lock (&tx_lock);
    new_tx_id = ++transaction_id;
    // pthread_mutex_unlock (&tx_lock);

    return new_tx_id;
}
