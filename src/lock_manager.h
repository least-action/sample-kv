#ifndef __KV_LOCK_MANAGER_H__
#define __KV_LOCK_MANAGER_H__

#include <stdlib.h>

#include "lock_rwlock.h"

struct kv_lm;

struct kv_lm* kv_lm_create (void);
int kv_lm_destruct (struct kv_lm *lm);

struct kv_rwl* kv_lm_get_rwlock (struct kv_lm *lm, char *key, size_t key_len);

#endif
