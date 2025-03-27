#ifndef __KV_SNAPSHOT_H__
#define __KV_SNAPSHOT_H__

#include <pthread.h>
#include <stdbool.h>

struct kv_snapshot_arg {
    pthread_cond_t *terminate_cond;
    pthread_mutex_t *terminate_lock;
};

void* kv_snapshot_thread (void *data);

#endif
