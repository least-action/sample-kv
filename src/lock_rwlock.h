#ifndef __KV_RW_LOCK_H__
#define __KV_RW_LOCK_H__

struct kv_rwl;

struct kv_rwl* kv_rwl_create ();
int kv_rwl_destroy (struct kv_rwl *rwl);

int kv_rwl_rlock (struct kv_rwl *rwl);
int kv_rwl_wlock (struct kv_rwl *rwl);
int kv_rwl_unlock (struct kv_rwl *rwl);

#endif
