#include "kv_hash.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

struct kv_ht_elem {
    void *key;
    void *value;
    struct kv_ht_elem *next;
};

struct kv_ht {
    size_t size;
    size_t count;
    hash_func_t hash_func;
    cmp_func_t cmp_func;
    struct kv_ht_elem **table;
};


struct kv_ht* kv_ht_create (int size, hash_func_t hash_func, cmp_func_t cmp_func)
{
    struct kv_ht *ht;
    ht = (struct kv_ht *) malloc (sizeof (struct kv_ht));
    assert (size > 0);
    ht->size = size;
    ht->count = 0;
    ht->hash_func = hash_func;
    ht->cmp_func = cmp_func;
    ht->table = (struct kv_ht_elem **) malloc (sizeof (struct kv_ht_elem *) * ht->size);
    memset (ht->table, '\0', sizeof (struct kv_ht_elem *) * ht->size);
    return ht;
}

int kv_ht_destroy (struct kv_ht *ht)
{
    // todo
    // todo: add this function call in every create called
    return 0;
}

static struct kv_ht_elem* kv_ht_get_elem (struct kv_ht *ht, void *key)
{
    struct kv_ht_elem *elem;
    size_t hash;

    hash = ht->hash_func (key);
    hash %= ht->size;

    elem = ht->table[hash];
    while (elem != NULL) {
        if (ht->cmp_func (elem->key, key) == 0)
            return elem;
        elem = elem->next;
    }

    return NULL;
}

void* kv_ht_get (struct kv_ht *ht, void *key)
{
    struct kv_ht_elem *elem;
    elem = kv_ht_get_elem (ht, key);
    return elem != NULL ? elem->value : NULL;
}

static void kv_ht_resize (struct kv_ht *ht, size_t size)
{
    struct kv_ht_elem **new_table;
    struct kv_ht_elem *target;
    struct kv_ht_elem *last_elem;
    size_t hash;

    new_table = (struct kv_ht_elem **) malloc (sizeof (struct kv_ht_elem *) * size);
    for (int i = 0; i < ht->size; ++i) {
        while (ht->table[i] != NULL) {
            hash = ht->hash_func (ht->table[i]->key);
            hash %= size;
            target = ht->table[i];
            
            if (new_table[hash] == NULL) {
                new_table[hash] = target;
            }
            else {
                last_elem = new_table[hash];
                while (last_elem->next != NULL)
                    last_elem = last_elem->next;
                last_elem->next = target;
            }
            ht->table[i] = target->next;
            target->next = NULL;
        }
    }

    free (ht->table);
    ht->table = new_table;
    ht->size = size;
}

void* kv_ht_set (struct kv_ht *ht, void *key, void *value)
{
    struct kv_ht_elem *elem;
    struct kv_ht_elem *new_elem;
    struct kv_ht_elem *last_elem;
    size_t hash;
    void *old_value;

    // todo: lock
    elem = kv_ht_get_elem (ht, key);
    if (elem != NULL) {
        old_value = elem->value;
        elem->value = value;
        return old_value;
    }
    if (ht->count > ht->size) {
        kv_ht_resize (ht, ht->size << 1);
    }
    
    new_elem = malloc (sizeof (struct kv_ht_elem));
    new_elem->key = key;
    new_elem->value = value;
    new_elem->next = NULL;

    hash = ht->hash_func (key);
    hash %= ht->size;

    if (ht->table[hash] == NULL) {
        ht->table[hash] = new_elem;
    }
    else {
        last_elem = ht->table[hash];
        while (last_elem->next != NULL)
            last_elem = last_elem->next;
        last_elem->next = new_elem;
    }

    ++(ht->count);
    return NULL;
}

struct kv_ht_kv kv_ht_del (struct kv_ht *ht, void *key)
{
    size_t hash;
    struct kv_ht_kv old_data;
    old_data.key = NULL;
    old_data.value = NULL;
    struct kv_ht_elem *elem;
    struct kv_ht_elem *temp;

    hash = ht->hash_func (key);
    hash %= ht->size;

    elem = ht->table[hash];
    if (elem == NULL)
        return old_data;
    
    if (ht->cmp_func (elem->key, key) == 0) {
        old_data.key = elem->key;
        old_data.value = elem->value;
        ht->table[hash] = elem->next;
        free (elem);
        --(ht->count);
        return old_data;
    }
    
    else {
        while (elem != NULL && elem->next != NULL) {
            if (ht->cmp_func (elem->next->key, key) == 0) {
                old_data.key = elem->next->key;
                old_data.value = elem->next->value;
                temp = elem->next;
                elem->next = elem->next->next;
                free (temp);
                --(ht->count);
                return old_data;
            }
            elem = elem->next;
        }
    }
    return old_data;
}

void kv_ht_foreach (struct kv_ht *ht, foreach_func_t foreach_func)
{
    struct kv_ht_elem *elem;
    struct kv_ht_kv kv;

    for (size_t pos = 0; pos < ht->size; ++pos) {
        elem = ht->table[pos];
        while (elem != NULL) {
            kv.key = elem->key;
            kv.value = elem->value;
            foreach_func (kv);
            elem = elem->next;
        }
    }
}
