#include "lock_rwlock.h"
#include "linked_list.h"

#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>

// todo: implement rwlock

struct kv_rwl {
    pthread_mutex_t lock;
    pthread_t owner;
    int lock_count;
};

struct kv_rwl* kv_rwl_create ()
{
    struct kv_rwl *rwl;
    rwl = (struct kv_rwl *) malloc (sizeof (struct kv_rwl));
    pthread_mutex_init (&rwl->lock, NULL);
    rwl->owner = 0;  // todo: use proper thread_t data
    rwl->lock_count = 0;
    return rwl;
}


int kv_rwl_destroy (struct kv_rwl *rwl)
{
    pthread_mutex_destroy (&rwl->lock);
    free (rwl);
    return 0;
}

int kv_rwl_rlock (struct kv_rwl *rwl)
{
    pthread_t self = pthread_self ();

    if (pthread_equal (rwl->owner, self)) {
        ++rwl->lock_count;
        printf("%lu get count %d\n", self, rwl->lock_count);
        return 0;
    }
    
    pthread_mutex_lock (&rwl->lock);
    rwl->lock_count = 1;
    rwl->owner = self;
    printf("%lu get count %d\n", self, rwl->lock_count);

    return 0;
}


int kv_rwl_wlock (struct kv_rwl *rwl)
{
    pthread_t self = pthread_self ();

    if (pthread_equal (rwl->owner, self)) {
        ++rwl->lock_count;
        printf("%lu get count %d\n", self, rwl->lock_count);
        return 0;
    }
    
    pthread_mutex_lock (&rwl->lock);
    rwl->lock_count = 1;
    printf("%lu get count %d\n", self, rwl->lock_count);
    rwl->owner = self;

    return 0;
}

int kv_rwl_unlock (struct kv_rwl *rwl)
{
    pthread_t self = pthread_self ();
    printf("%lu unlock\n", self);

    if (pthread_equal (rwl->owner, self)) {
        --rwl->lock_count;
        printf("%lu lock count: %d\n", self, rwl->lock_count);
        if (rwl->lock_count == 0) {
            rwl->owner = 0;
            printf("%lu unlock lock\n", self);
            pthread_mutex_unlock (&rwl->lock);
        }
    }

    return 0;
}
