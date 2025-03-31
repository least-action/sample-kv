#ifndef __KV_COMMAND_H__
#define __KV_COMMAND_H__

#include "kv_hash.h"
#include "lock_manager.h"

#include <stddef.h>
#include <stdint.h>


int consume_command(struct kv_ht *ht, struct kv_lm *lm, char *command, char *result, int *tx_id);

#endif
