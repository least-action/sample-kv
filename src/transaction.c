#include "transaction.h"
#include "lock_rwlock.h"

#include <stdlib.h>
#include <stdint.h>

struct lock_elem {
    struct kv_rwl *rwl;
    struct lock_elem *next;
};

struct kv_tx {
    uint32_t id;
    struct lock_elem *head;
    uint32_t prev_lsn;
};

struct kv_tx* kv_tx_create (int tx_id)
{
    struct kv_tx *tx;
    tx = (struct kv_tx *) malloc (sizeof (struct kv_tx));
    tx->id = tx_id;
    tx->prev_lsn = 0;

    return tx;
}

int kv_tx_destroy (struct kv_tx *tx)
{
    free (tx);
    return 0;
}

uint32_t kv_tx_get_id (struct kv_tx *tx)
{
    return tx->id;
}

uint32_t kv_tx_prev_lsn (struct kv_tx *tx)
{
    return tx->prev_lsn;
}

int kv_tx_set_prev_lsn (struct kv_tx *tx, uint32_t new_lsn)
{
    tx->prev_lsn = new_lsn;
    return 0;
}

int kv_tx_rollback (struct kv_tx *tx)
{
    // todo
    return 0;
}