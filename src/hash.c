#include "hash.h"
#include "linked_list.h"
#include <string.h>

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
char* set_result = "OK";
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

bool elem_is_equal(void *elem, void *key)
{
    
    return strcmp(((struct kv_hash_elem *)elem)->key, (char *)key) == 0;
}

char* hash_get_value(struct kv_hash_table *table, char* key)
{
    unsigned long hash_value = get_hash(key);
    unsigned long hash = hash_value % table->size; 

    struct kv_linked_list *ll;
    struct kv_hash_elem *elem;

    ll = table->ll_list[hash];
    if (ll == NULL)
        return get_notfound;
    elem = (struct kv_hash_elem *) kv_ll_find (ll, key, elem_is_equal);
    if (elem == NULL)
        return get_notfound;
    return elem->value;
}

char* hash_set_value(struct kv_hash_table *table, char* key, char* value)
{
    return set_result;
}

char* hash_del_value(struct kv_hash_table *table, char* key)
{
    return del_success;
}

