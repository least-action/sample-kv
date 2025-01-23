#include "hash.h"
#include "linked_list.h"
#include <string.h>
#include <stdio.h>

struct kv_hash_elem {
    char* key;
    char* value;
};

struct kv_hash_table {
    unsigned long size;
    struct kv_linked_list **ll_list;
};


struct kv_hash_table ht;

char* get_notfound = "(nil)";
char* set_success = "OK";
char* del_success = "Deleted";
char* del_not_found = "Not found";

unsigned long djb2(char* str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;

    return hash;
}

void hash_init_table(struct kv_hash_table *table)
{
    table->size = 1024;
    table->ll_list = (struct kv_linked_list **) malloc (sizeof (void*) * table->size);
    memset(table->ll_list, '\0', table->size);
}

unsigned long get_hash(char* str)
{
    return djb2(str);
}

bool elem_is_equal(void *key, void *elem)
{
    
    return strcmp(
        (char *) key,
        ((struct kv_hash_elem *)elem)->key
    ) == 0;
}

char* hash_get_value(struct kv_hash_table *table, char* key)
{
    unsigned long hash_value = get_hash(key);
    unsigned long hash = hash_value % table->size; 

    struct kv_linked_list *ll;
    struct kv_hash_elem *elem;

    ll = table->ll_list[hash];
    if (ll == NULL) {
        printf("1\n");
        return get_notfound;
    }
    elem = (struct kv_hash_elem *) kv_ll_find (ll, key, elem_is_equal);
    if (elem == NULL) {
        printf("2\n");
        return get_notfound;
    }
    return elem->value;
}

char* hash_set_value(struct kv_hash_table *table, char* key, char* value)
{
    unsigned long hash_value = get_hash(key);
    unsigned long hash = hash_value % table->size; 

    struct kv_linked_list *ll;
    struct kv_hash_elem *elem;
    char *new_value;
    size_t value_size = strlen(value) + 1;
    new_value = (char *) malloc (value_size);
    memcpy(new_value, value, value_size);
    char *new_key;
    size_t key_size = strlen(key) + 1;
    new_key = (char *) malloc (key_size);
    memcpy(new_key, key, key_size);
    struct kv_hash_elem new_elem;

    ll = table->ll_list[hash];
    if (ll == NULL) {
        ll = (struct kv_linked_list *) malloc (kv_ll_sizeof ());
        kv_ll_init (ll);
        table->ll_list[hash] = ll;
    }

    elem = (struct kv_hash_elem *) kv_ll_find (ll, key, elem_is_equal);
    if (elem != NULL) {
        char* del_key = elem->key;
        char* del_value = elem->value;
        kv_ll_del (ll, key, elem_is_equal);
        free (del_key);
        free (del_value);
    }

    new_elem.key = new_key;
    new_elem.value = new_value;
    kv_ll_add (ll, &new_elem, sizeof(new_elem));

    return set_success;
    
    
}

char* hash_del_value(struct kv_hash_table *table, char* key)
{
    return del_success;
}

