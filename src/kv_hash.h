#ifndef __KV_HASH_H__
#define __KV_HASH_H__

#include <stddef.h>

struct kv_ht;
struct kv_ht_kv {
    void *key;
    void *value;
};
typedef size_t (*hash_func_t) (const void *key);
typedef int (*cmp_func_t) (const void *key, const void *key2);

struct kv_ht* kv_ht_create (int size, hash_func_t hash_fund, cmp_func_t cmp_func);

int kv_ht_destroy (struct kv_ht *ht);

void* kv_ht_get (struct kv_ht *ht, void *key);

/* old value ptr: updated, NULL: inserted new key */
void* kv_ht_set (struct kv_ht *ht, void *key, void *value);

/* 0: key not found, 1: deleted */
struct kv_ht_kv kv_ht_del (struct kv_ht *ht, void *key);

typedef void (*foreach_func_t) (const struct kv_ht_kv kv);
void kv_ht_foreach (struct kv_ht *ht, foreach_func_t foreach_func);

#endif
