#include "linked_list.h"

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <stddef.h>


struct kv_ll_elem {
    void* data;
    struct kv_ll_elem *next;
    struct kv_ll_elem *prev;
};

struct kv_ll {
    is_equal *equal_func;
    size_t size;
    struct kv_ll_elem *head;
    struct kv_ll_elem *tail;
    pthread_mutex_t lock;
};

struct kv_ll* kv_ll_create (is_equal *equal_func)
{
    struct kv_ll *ll = (struct kv_ll *) malloc (sizeof (struct kv_ll));
    ll->equal_func = equal_func;
    ll->size = 0;
    ll->head = NULL;
    ll->tail = NULL;
    pthread_mutex_init (&ll->lock, NULL);
    return ll;
}

int kv_ll_destroy (struct kv_ll *ll)
{
    if (ll->head != NULL)
        return 1;
    
    pthread_mutex_destroy (&ll->lock);
    free (ll);
    return 0;
}

size_t kv_ll_size (struct kv_ll *ll)
{
    return ll->size;
}

void kv_ll_add (struct kv_ll *ll, void *data)
{
    struct kv_ll_elem *new_elem = (struct kv_ll_elem *) malloc (sizeof (struct kv_ll_elem));
    new_elem->data = data;
    new_elem->next = NULL;

    pthread_mutex_lock (&ll->lock);
    {
        if (ll->head == NULL) {
            new_elem->prev = NULL;
            ll->head = new_elem;
            ll->tail = new_elem;
        } else {
            ll->tail->next = new_elem;
            ll->tail = new_elem;
        }
        ++ll->size;
    }
    pthread_mutex_unlock (&ll->lock);
}

void* kv_ll_del (struct kv_ll *ll, void *data)
{
    struct kv_ll_elem *elem = ll->head;
    void *del_data_ptr = NULL;

    pthread_mutex_lock (&ll->lock);
    {
        for (elem = ll->head; elem != NULL; elem = elem->next) {
            if (!(ll->equal_func (data, elem->data)))
                continue;

            del_data_ptr = elem->data;
            if (elem == ll->head && elem == ll->tail) {
                ll->head = NULL;
                ll->tail = NULL;
            } else if (elem == ll->head) {
                ll->head = elem->next;
                if (ll->head != NULL)
                    ll->head->prev = NULL;
            } else if (elem == ll->tail) {
                ll->tail = elem->prev;
                if (ll->tail != NULL)
                    ll->tail->next = NULL;
            } else {
                elem->prev->next = elem->next;
                elem->next->prev = elem->prev;
            }
            free (elem);
            --ll->size;
            break;
        }
    }
    pthread_mutex_unlock (&ll->lock);

    return del_data_ptr;
}

void *kv_ll_del_tail (struct kv_ll *ll)
{
    struct kv_ll_elem *elem;
    void *data;

    pthread_mutex_lock (&ll->lock);
    {
        if (ll->head == NULL)
            data = NULL;
        else if (ll->head == ll->tail) {
            elem = ll->tail;
            data = elem->data;
            free (elem);
            --ll->size;

            ll->head = NULL;
            ll->tail = NULL;
        } else {
            elem = ll->tail;
            data = elem->data;
            ll->tail->prev->next = NULL;
            free (elem);
            --ll->size;
        }
    }
    pthread_mutex_unlock (&ll->lock);

    return data;
}

void kv_ll_foreach (struct kv_ll *ll, void func (void*, void*), void *param)
{
    struct kv_ll_elem *elem = ll->head;

    pthread_mutex_lock (&ll->lock);
    {
        for (elem = ll->head; elem != NULL; elem = elem->next)
            func (param, elem->data);
    }
    pthread_mutex_unlock (&ll->lock);
}
