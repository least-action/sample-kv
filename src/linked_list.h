#ifndef __KV_LINKED_LIST_H__
#define __KV_LINKED_LIST_H__

#include <stdbool.h>
#include <stdlib.h>

struct kv_linked_list_elem;
struct kv_linked_list;

size_t kv_ll_sizeof (void);
size_t kv_ll_count (struct kv_linked_list *ll);

void kv_ll_init (struct kv_linked_list *ll);
void* kv_ll_find (struct kv_linked_list *ll, void *data, bool (*is_equal) (void *, void*));
void kv_ll_add (struct kv_linked_list *ll, void *void_data, size_t data_size);
int kv_ll_del (struct kv_linked_list *ll, void *data, bool (*is_equal) (void *, void*));

#endif

