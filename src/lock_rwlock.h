#ifndef __KV_RW_LOCK_H__
#define __KV_RW_LOCK_H__

struct kv_rwl;

int kv_rwl_rlock (struct kv_rwl *rwl);
int kv_rwl_un_rlock (struct kv_rwl *rwl);

int kv_rwl_wlock (struct kv_rwl *rwl);
int kv_rwl_un_wlock (struct kv_rwl *rwl);


#endif
