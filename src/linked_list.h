#ifndef __KV_LINKED_LIST_H__
#define __KV_LINKED_LIST_H__

#include <stdbool.h>
#include <stdlib.h>

typedef bool is_equal (void *a, void *b);
struct kv_ll;

struct kv_ll* kv_ll_create (is_equal *equal_func);
int kv_ll_destroy (struct kv_ll *ll);

size_t kv_ll_size (struct kv_ll *ll);
void kv_ll_add (struct kv_ll *ll, void *data);
// todo: refactor: require key only for del and get 
void* kv_ll_del (struct kv_ll *ll, void *data);
void *kv_ll_del_tail (struct kv_ll *ll);
void kv_ll_foreach (struct kv_ll *ll, void func (void*, void*), void *param);

#endif
