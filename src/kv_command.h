#ifndef __KV_COMMAND_H__
#define __KV_COMMAND_H__

#include "kv_hash.h"
#include <stddef.h>

int consume_command(struct kv_ht *ht, char *buffer, char *result, int *transaction_id);

#endif

