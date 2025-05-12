#include "lock_rwlock.h"
#include "linked_list.h"

#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>

// todo: thread_t id manupulation logic is only adopted for [unsigned long] type.
// todo: implement rwlock

typedef unsigned long th_id;
static th_id const EMPTH_TID = 0;

static struct wait_elem {
    th_id tid;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    bool is_waken;
    struct wait_elem *prev;
    struct wait_elem *next;
};

struct kv_rwl {
    pthread_mutex_t lock;
    th_id owner_tid;
    struct wait_elem *wait_head;
    struct wait_elem *wait_tail;
};

struct kv_rwl* kv_rwl_create ()
{
    struct kv_rwl *rwl;
    rwl = (struct kv_rwl *) malloc (sizeof (struct kv_rwl));
    pthread_mutex_init (&rwl->lock, NULL);
    // pthread_mutex_init (&rwl->data_lock, NULL);
    rwl->owner_tid = EMPTH_TID;
    rwl->wait_head = NULL;
    rwl->wait_tail = NULL;
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
    unsigned long tid = (unsigned long) pthread_self ();
    struct wait_elem *new_elem;
    bool is_waited = false;
    
    pthread_mutex_lock (&rwl->lock);
    {
        if (rwl->owner_tid == tid) {
            // pass
        } else if (rwl->owner_tid == EMPTH_TID) {
            // pthread_mutex_lock (&rwl->data_lock);
            rwl->owner_tid = tid;
        } else {
            is_waited = true;
            
            new_elem = (struct wait_elem *) malloc (sizeof (struct wait_elem));
            new_elem->tid = tid;
            pthread_mutex_init (&new_elem->mutex, NULL);
            pthread_cond_init (&new_elem->cond, NULL);
            new_elem->next = NULL;
            new_elem->is_waken = false;

            if (rwl->wait_head == NULL) {
                new_elem->prev = NULL;
                rwl->wait_head = new_elem;
                rwl->wait_tail = new_elem;
            } else {
                new_elem->prev = rwl->wait_tail;
                rwl->wait_tail->next = new_elem;
                rwl->wait_tail = new_elem;
            }

            pthread_mutex_lock (&new_elem->mutex);
        }
    }
    pthread_mutex_unlock (&rwl->lock);

    if (is_waited) {
        {
            while (!new_elem->is_waken)
                pthread_cond_wait (&new_elem->cond, &new_elem->mutex);
        }
        pthread_mutex_unlock (&new_elem->mutex);
    }

    return 0;
}


int kv_rwl_wlock (struct kv_rwl *rwl)
{
    unsigned long tid = (unsigned long) pthread_self ();
    struct wait_elem *new_elem;
    bool is_waited = false;
    
    pthread_mutex_lock (&rwl->lock);
    {
        if (rwl->owner_tid == tid) {
            // pass
        } else if (rwl->owner_tid == EMPTH_TID) {
            // pthread_mutex_lock (&rwl->data_lock);
            rwl->owner_tid = tid;
        } else {
            is_waited = true;
            
            new_elem = (struct wait_elem *) malloc (sizeof (struct wait_elem));
            new_elem->tid = tid;
            pthread_mutex_init (&new_elem->mutex, NULL);
            pthread_cond_init (&new_elem->cond, NULL);
            new_elem->next = NULL;
            new_elem->is_waken = false;

            if (rwl->wait_head == NULL) {
                new_elem->prev = NULL;
                rwl->wait_head = new_elem;
                rwl->wait_tail = new_elem;
            } else {
                new_elem->prev = rwl->wait_tail;
                rwl->wait_tail->next = new_elem;
                rwl->wait_tail = new_elem;
            }

            pthread_mutex_lock (&new_elem->mutex);
        }
    }
    pthread_mutex_unlock (&rwl->lock);

    if (is_waited) {
        {
            while (!new_elem->is_waken)
                pthread_cond_wait (&new_elem->cond, &new_elem->mutex);
        }
        pthread_mutex_unlock (&new_elem->mutex);
    }
    

    return 0;
}

int kv_rwl_unlock (struct kv_rwl *rwl)
{
    unsigned long tid = (unsigned long) pthread_self ();
    struct wait_elem *waking_elem;
    
    pthread_mutex_lock (&rwl->lock);
    {
        if (rwl->owner_tid == tid) {
            rwl->owner_tid = EMPTH_TID;
            // pthread_mutex_unlock (&rwl->data_lock);
            if (rwl->wait_head == NULL) {
                // pass
            } else {
                
                waking_elem = rwl->wait_head;
                rwl->wait_head = waking_elem->next;
                if (rwl->wait_head == NULL) {
                    rwl->wait_tail = NULL;
                }

                rwl->owner_tid = waking_elem->tid;
                    
                pthread_mutex_lock (&waking_elem->mutex);
                {
                    waking_elem->is_waken = true;
                    pthread_cond_signal (&waking_elem->cond);
                }
                pthread_mutex_unlock (&waking_elem->mutex);
            }
        }
    }
    pthread_mutex_unlock (&rwl->lock);

    return 0;
}
