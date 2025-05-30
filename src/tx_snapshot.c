// #include "tx_snapshot.h"

// #include "shutdown.h"

// #include <time.h>
// #include <errno.h>
// #include <stdio.h>

// static const time_t DELAY = 5;

// void* kv_tx_snapshot_handler (void *data)
// {
//     struct kv_tx_snapshot_arg *arg = (struct kv_tx_snapshot_arg *) data;
//     pthread_cond_t *cond = arg->terminate_cond;
//     pthread_mutex_t *lock = arg->terminate_lock;
//     int res;
//     struct timespec ts;

//     pthread_mutex_lock (lock);
//     while (is_running) {
//         clock_gettime (CLOCK_REALTIME, &ts);
//         ts.tv_sec += DELAY;
//         res = pthread_cond_timedwait (cond, lock, &ts);
//         if (res != ETIMEDOUT) {
//             printf ("tx get cond\n");
//             continue;
//         }

//         printf ("tx snapshot!\n");
//     }
//     pthread_mutex_unlock (lock);
//     return NULL;
// }
