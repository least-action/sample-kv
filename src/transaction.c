#include "transaction.h"
#include "lock_rwlock.h"

#include <stdlib.h>

static struct lock_elem {
    struct kv_rwl *rwl;
    struct lock_elem *next;
};

struct kv_tx {
    int id;
    struct lock_elem *head;
};

struct kv_tx* kv_tx_create (int tx_id)
{
    struct kv_tx *tx;
    tx = (struct kv_tx *) malloc (sizeof (struct kv_tx));
    tx->id = tx_id;

    return tx;
}

int kv_tx_destroy (struct kv_tx *tx)
{
    free (tx);
    return 0;
}

int kv_tx_get_id (struct kv_tx *tx)
{
    return tx->id;
}
