#ifndef __KV_TX_SNAPSHOT_H__
#define __KV_TX_SNAPSHOT_H__

#include <pthread.h>

struct kv_tx_snapshot_arg {
    pthread_cond_t *terminate_cond;
    pthread_mutex_t *terminate_lock;
};

void* kv_tx_snapshot_handler (void *data);


#endif
