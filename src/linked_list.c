#include "linked_list.h"

#include <stdlib.h>
#include <stdio.h>
struct kv_ll_elem {
    void* data;
    struct kv_ll_elem *next;
    struct kv_ll_elem *prev;
};

struct kv_ll {
    is_equal *equal_func;
    struct kv_ll_elem *head;
    struct kv_ll_elem *tail;
};

struct kv_ll* kv_ll_create (is_equal *equal_func)
{
    struct kv_ll *ll = (struct kv_ll *) malloc (sizeof (struct kv_ll));
    ll->equal_func = equal_func;
    ll->head = NULL;
    ll->tail = NULL;
    return ll;
}

void kv_ll_add (struct kv_ll *ll, void *data)
{
    struct kv_ll_elem *new_elem = (struct kv_ll_elem *) malloc (sizeof (struct kv_ll_elem));
    new_elem->data = data;
    new_elem->next = NULL;

    if (ll->head == NULL) {
        new_elem->prev = NULL;
        ll->head = new_elem;
        ll->tail = new_elem;
    } else {
        ll->tail->next = new_elem;
        ll->tail = new_elem;
    }
}

void* kv_ll_del (struct kv_ll *ll, void *data)
{
    struct kv_ll_elem *elem = ll->head;
    void *del_data_ptr = NULL;
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
        break;
    }

    return del_data_ptr;
}

void kv_ll_foreach (struct kv_ll *ll, void func (void*, void*), void *param)
{
    struct kv_ll_elem *elem = ll->head;
    for (elem = ll->head; elem != NULL; elem = elem->next)
        func (param, elem->data);
}
