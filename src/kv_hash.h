#ifndef __KV_HASH_H__
#define __KV_HASH_H__

struct kv_ht;

struct kv_ht* kv_ht_create (int size);

char* kv_ht_get (struct kv_ht *ht, char *key);

/* 0: updated, 1: inserted new key */
int kv_ht_set (struct kv_ht *ht, char *key, char *value);

/* 0: key not found, 1: deleted */
int kv_ht_del (struct kv_ht *ht, char *key);

#endif

