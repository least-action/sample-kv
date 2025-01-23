#include "linked_list.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct kv_linked_list_elem {
    void *data;
    struct kv_linked_list_elem *prev;
    struct kv_linked_list_elem *next;
};

struct kv_linked_list {
    struct kv_linked_list_elem *head;
    struct kv_linked_list_elem *tail;
};

void kv_ll_init(struct kv_linked_list *ll)
{
    ll->head = NULL;
    ll->tail = NULL;
}

bool kv_ll_is_empty(struct kv_linked_list *ll)
{
    return ll->head == NULL;
}

void* kv_ll_find_elem(struct kv_linked_list *ll, void *data, bool (*is_equal) (void *, void*))
{
    struct kv_linked_list_elem *cur;
    cur = ll->head;
    while (cur != NULL) {
        if (is_equal (data, cur->data))
            break;
        cur = cur->next;
    }   
        
    return cur;
}

void* kv_ll_find(struct kv_linked_list *ll, void *data, bool (*is_equal) (void *, void*))
{
    struct kv_linked_list_elem *elem;
    elem = kv_ll_find_elem (ll, data, is_equal);

    return elem == NULL ? NULL : elem->data;
}

void kv_ll_add(struct kv_linked_list *ll, void *void_data, size_t data_size)
{
    struct kv_linked_list_elem *elem =
        (struct kv_linked_list_elem *) malloc (sizeof (struct kv_linked_list_elem));
    void *data = malloc (data_size);
    memcpy(data, void_data, data_size);
    elem->data = data;
    if (kv_ll_is_empty (ll)) {
        ll->head = elem;
        ll->tail = elem;
        elem->prev = NULL;
        elem->next = NULL;
    }   
    else {
        elem->prev = ll->tail;
        ll->tail->next = elem;
        ll->tail = elem;
    }   
}

void kv_ll_del(struct kv_linked_list *ll, void *data, bool (*is_equal) (void *, void*))
{
    struct kv_linked_list_elem *elem;
    elem = kv_ll_find_elem (ll, data, is_equal);
    if (elem == NULL)
        return;

    if (elem == ll->head && elem == ll->tail) {
        ll->head = NULL;
        ll->tail = NULL;
    }
    else if (elem == ll->head) {
        ll->head = elem->next;
        ll->head->prev = NULL;
    }
    else if (elem == ll->tail) {
        ll->tail = elem->prev;
        ll->tail->next = NULL;
    }
    else {
        elem->prev->next = elem->next;
        elem->next->prev = elem->prev;
    }
    free (elem->data);
    free (elem);
}

