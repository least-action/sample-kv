#include "lock_manager.h"
#include "kv_hash.h"

#include <pthread.h>

struct kv_lm
{
    struct kv_ht* ht;
    pthread_mutex_t ht_lock;  // todo: refactor: add kv_ht_get_or_set ?
};

static size_t hash_func (const void *key)
{
    return 0;
}

static int cmp_func (const void *key, const void *key2)
{
    return 0;
}


struct kv_lm* kv_lm_create (void)
{
    struct kv_lm *lm = (struct kv_lm *) malloc (sizeof (struct kv_lm));
    lm->ht = kv_ht_create (2, hash_func, cmp_func);
    pthread_mutex_init (&lm->ht_lock, NULL);
    return lm;
}

int kv_lm_destroy (struct kv_lm *lm)
{
    // todo: destroy rwl
    kv_ht_destroy (lm->ht);
    pthread_mutex_destroy (&lm->ht_lock);
    free (lm);
    return 0;
}

// todo: perf: add remove rwlock process
struct kv_rwl* kv_lm_get_rwlock (struct kv_lm *lm, char *key, size_t key_len)
{
    // todo
    return NULL;
}
