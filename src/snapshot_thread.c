#include "snapshot_thread.h"
#include "shutdown.h"

#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <errno.h>
#include <stdio.h>

static const time_t DELAY = 1;

void* kv_snapshot_thread (void *data)
{
    struct kv_snapshot_arg *arg = (struct kv_snapshot_arg *) data;
    pthread_cond_t *cond = arg->terminate_cond;
    pthread_mutex_t *lock = arg->terminate_lock;
    int res;
    struct timespec ts;

    pthread_mutex_lock (lock);
    while (is_running) {
        clock_gettime (CLOCK_REALTIME, &ts);
        ts.tv_sec += DELAY;
        res = pthread_cond_timedwait (cond, lock, &ts);
        if (res != ETIMEDOUT) {
            printf ("get cond\n");
            continue;
        }

        printf ("snapshot!\n");
    }
    pthread_mutex_unlock (lock);
    return NULL;
}
