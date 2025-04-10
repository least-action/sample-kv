#include "lock_rwlock.h"

#include <stdlib.h>
#include <pthread.h>

struct kv_rwl {
    pthread_rwlock_t lock;
};

struct kv_rwl* kv_rwl_create ()
{
    struct kv_rwl *rwl;
    rwl = (struct kv_rwl *) malloc (sizeof (struct kv_rwl));
    pthread_rwlock_init (&rwl->lock, NULL);
    return rwl;
}

int kv_rwl_destroy (struct kv_rwl *rwl)
{
    pthread_rwlock_destroy (&rwl->lock);
    free (rwl);
    return 0;
}

int kv_rwl_rlock (struct kv_rwl *rwl)
{
    pthread_rwlock_rdlock (&rwl->lock);
    return 0;
}


int kv_rwl_wlock (struct kv_rwl *rwl)
{
    pthread_rwlock_wrlock (&rwl->lock);
    return 0;
}

int kv_rwl_unlock (struct kv_rwl *rwl)
{
    pthread_rwlock_unlock (&rwl->lock);
    return 0;
}
