#ifndef __KV_COMMAND_H__
#define __KV_COMMAND_H__

#include "kv_hash.h"

#include <stddef.h>
#include <stdint.h>


int consume_command(struct kv_ht *ht, char *command, char *result, int *tx_id);

size_t str_hash_func (const void *key);
int str_cmp_func (const void *key1, const void *key2);

#endif
