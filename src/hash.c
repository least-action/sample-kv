#include "hash.h"
#include "linked_list.h"

struct kv_hash_elem {
    char* key;
    char* value;
};

struct kv_hash_table {
    unsigned long size;
    
};


struct kv_hash_table ht;

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
}

unsigned long get_hash(char* str)
{
    return djb2(str);
}

char* hash_get_value(struct kv_hash_table *table, char* key)
{
    unsigned long hash_value = get_hash(key);
    unsigned long hash = hash_value % table->size; 

    return set_result;
}

char* hash_set_value(struct kv_hash_table *table, char* key, char* value)
{
    return set_result;
}

char* hash_del_value(struct kv_hash_table *table, char* key)
{
    return del_success;
}

