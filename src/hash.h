#ifndef __KV_HASH_H__
#define __KV_HASH_H__

struct kv_hash_table;

extern struct kv_hash_table ht;

void hash_init_table(struct kv_hash_table *table);

char* hash_get_value(struct kv_hash_table *table, char* key);

char* hash_set_value(struct kv_hash_table *table, char* key, char* value);

char* hash_del_value(struct kv_hash_table *table, char* key);

#endif

