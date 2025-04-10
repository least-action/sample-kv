#include "lock_manager.h"
#include "kv_hash.h"
#include "lock_rwlock.h"
#include "utils.h"

#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>

static const size_t default_list_size = 64;

struct kv_lm
{
    pthread_rwlock_t *lock_list;
    size_t size;
    atomic_long counter;
    pthread_rwlock_t lock;  // for resizing
    // todo: resizing function should be called independently to avoid redundant resizing
};

struct kv_lm* kv_lm_create (void)
{
    struct kv_lm *lm = (struct kv_lm *) malloc (sizeof (struct kv_lm));
    lm->size = default_list_size;
    lm->counter = ATOMIC_VAR_INIT (0);
    lm->lock_list = (pthread_rwlock_t *) malloc (sizeof (pthread_rwlock_t) * lm->size);
    for (int i = 0; i < lm->size; ++i) {
        pthread_rwlock_init (&lm->lock_list[i], NULL);
    }
    return lm;
}

int kv_lm_destroy (struct kv_lm *lm)
{
    for (int i = 0; i < lm->size; ++i) {
        pthread_rwlock_destroy (&lm->lock_list[i]);
    }
    free (lm);
    return 0;
}

int kv_lm_rlock (struct kv_lm *lm, char *key, size_t key_len)
{
    size_t hash, idx;

    hash = djb2 (key, key_len);
    atomic_fetch_add (&lm->counter, 1);
    pthread_rwlock_rdlock (&lm->lock);
    {
        idx = hash % lm->size;
        pthread_rwlock_rdlock (&lm->lock_list[idx]);
    }
    pthread_rwlock_unlock (&lm->lock);

    return 0;
}

int kv_lm_wlock (struct kv_lm *lm, char *key, size_t key_len)
{
    size_t hash, idx;

    hash = djb2 (key, key_len);
    atomic_fetch_add (&lm->counter, 1);
    pthread_rwlock_rdlock (&lm->lock);
    {
        idx = hash % lm->size;
        pthread_rwlock_wrlock (&lm->lock_list[idx]);
    }
    pthread_rwlock_unlock (&lm->lock);

    return 0;
}

int kv_lm_unlock (struct kv_lm *lm, char *key, size_t key_len)
{
    size_t hash, idx;

    hash = djb2 (key, key_len);
    atomic_fetch_sub (&lm->counter, 1);  // todo: check position of this line
    pthread_rwlock_rdlock (&lm->lock);
    {
        idx = hash % lm->size;
        pthread_rwlock_unlock (&lm->lock_list[idx]);
    }
    pthread_rwlock_unlock (&lm->lock);

    return 0;
}
