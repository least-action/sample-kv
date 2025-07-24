#ifndef __KV_RECOVERY_DUMP_H__
#define __KV_RECOVERY_DUMP_H__

#include "kv_hash.h"

#include <pthread.h>

#define SNAPSHOT_DIR_NAME "snapshots"
#define LAST_SNAPSHOT_LSN_FILE "last_checked_lsn.kvdb"

// struct kv_snapshot_file {
//     char name[22];  // snapshot_00000000.kvdb
// };

struct kv_rec_snapshot_arg {
    pthread_cond_t *terminate_cond;
    pthread_mutex_t *terminate_lock;
    struct kv_ht *ht;
};

void* kv_recovery_snapshot_handler (void *data);

#endif
