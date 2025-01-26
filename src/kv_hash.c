#include "kv_hash.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

struct kv_ht_elem {
    char *key;
    char *value;
    struct kv_ht_elem *next;
};

struct kv_ht {
    unsigned long size;
    unsigned long count;
    struct kv_ht_elem **table;
};

// todo: remove
struct kv_ht ht;

unsigned long djb2(char* str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;

    return hash;
}

unsigned long get_hash(char* str)
{
    return djb2(str);
}

struct kv_ht* kv_ht_create (int size)
{
    struct kv_ht *ht;
    ht = (struct kv_ht *) malloc (sizeof (struct kv_ht));
    assert (size > 0);
    ht->size = size;
    ht->count = 0;
    ht->table = (struct kv_ht_elem **) malloc (sizeof (struct kv_ht_elem *) * ht->size);
    memset (ht->table, '\0', sizeof (struct kv_ht_elem *) * ht->size);
    return ht;
}

struct kv_ht_elem* kv_ht_get_elem (struct kv_ht *ht, char *key)
{
    unsigned long hash = get_hash (key);
    hash %= ht->size;

    struct kv_ht_elem *elem;
    elem = ht->table[hash];
    while (elem != NULL) {
        if (strcmp (elem->key, key) == 0)
            return elem;
        elem = elem->next;
    }
    return NULL;
}

char* kv_ht_get (struct kv_ht *ht, char *key)
{
    struct kv_ht_elem *elem;
    elem = kv_ht_get_elem (ht, key);
    return elem != NULL ? elem->value : NULL;
}

void kv_ht_resize (struct kv_ht *ht, unsigned long size)
{
    struct kv_ht_elem **new_table;
    struct kv_ht_elem *target;
    struct kv_ht_elem *last_elem;
    unsigned long hash;
    new_table = (struct kv_ht_elem **) malloc (sizeof (struct kv_ht_elem *) * size);
    for (int i = 0; i < ht->size; ++i) {
        while (ht->table[i] != NULL) {
            hash = get_hash (ht->table[i]->key);
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

int kv_ht_set (struct kv_ht *ht, char *key, char *value)
{
    char *new_value;
    int value_len = strlen (value) + 1;
    new_value = malloc (value_len);
    memcpy (new_value, value, value_len);

    struct kv_ht_elem *elem;
    elem = kv_ht_get_elem (ht, key);
    if (elem != NULL) {
        free (elem->value);
        elem->value = new_value;
        return 0;
    }
    if (ht->count > ht->size) {
        kv_ht_resize (ht, ht->size << 1);
    }
    
    char *new_key;
    int key_len = strlen (key) + 1;
    new_key = malloc (key_len);
    memcpy (new_key, key, key_len);
    
    struct kv_ht_elem *new_elem;
    new_elem = malloc (sizeof (struct kv_ht_elem));
    new_elem->key = new_key;
    new_elem->value = new_value;
    new_elem->next = NULL;

    unsigned long hash = get_hash (key);
    hash %= ht->size;

    struct kv_ht_elem *last_elem;
    last_elem = ht->table[hash];
    if (last_elem == NULL) {
        ht->table[hash] = new_elem;
    }
    else {
        while (last_elem->next != NULL)
            last_elem = last_elem->next;
        last_elem->next = new_elem;
    }

    ++(ht->count);
    return 1;
}

int kv_ht_del (struct kv_ht *ht, char *key)
{
    unsigned long hash = get_hash (key);
    hash %= ht->size;

    struct kv_ht_elem *elem;
    struct kv_ht_elem *temp;
    elem = ht->table[hash];
    if (elem == NULL)
        return 0;
    
    if (strcmp (elem->key, key) == 0) {
        free (elem->key);
        free (elem->value);
        ht->table[hash] = elem->next;
        free (elem);
        --(ht->count);
        return 1;
    }
    
    else {
        while (elem != NULL && elem->next != NULL) {
            if (strcmp (elem->next->key, key) == 0) {
                free (elem->next->key);
                free (elem->next->value);
                temp = elem->next;
                elem->next = elem->next->next;
                free (temp);
                --(ht->count);
                return 1;
            }
            elem = elem->next;
        }
    }
    return 0;
}


