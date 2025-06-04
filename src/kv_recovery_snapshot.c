#include "kv_recovery_snapshot.h"
#include "shutdown.h"
#include "kv_recovery.h"
#include "tx_manager.h"
#include "kv_hash.h"

#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>

static const time_t DELAY = 5;

void* kv_recovery_snapshot_handler (void *data)
{
    struct kv_rec_snapshot_arg *arg = (struct kv_rec_snapshot_arg *) data;
    pthread_cond_t *cond = arg->terminate_cond;
    pthread_mutex_t *lock = arg->terminate_lock;
    struct kv_ht *ht = arg->ht;
    int res;
    struct timespec ts;
    uint32_t *tx_id_list;
    size_t tx_id_list_size;

    pthread_mutex_lock (lock);
    while (is_running) {
        
        clock_gettime (CLOCK_REALTIME, &ts);
        ts.tv_sec += DELAY;
        res = pthread_cond_timedwait (cond, lock, &ts);
        if (res != ETIMEDOUT) {
            printf ("check snapshot get cond\n");
            continue;
        }

        printf ("check snapshot!\n");

        // lock > 이 lock 이 걸리면 해제될 때 까지 다른 transaction 은 시작되면 안됨. 종료도 되어선 안됨.
        // begin, commit, abort 등과 같이 transaction 이 시작되고 종료되는 작업을 하기 전에 log 를 먼저 작성해야함
        // begin 은 로그를 먼저 찍을 수 없음. tx 가 정해져야 찍을 수 있어서.
        tx_id_list = NULL;
        kv_txm_lock ();  // tx 목록을 뽑기 위해 새로 생성되거나 종료되지 못하게
        kv_recovery_lock ();
        kv_ht_lock (ht);
        {
            tx_id_list_size = kv_txm_ongoing_transaction_ids (&tx_id_list);
            kv_recovery_check_log (tx_id_list, tx_id_list_size);
        }
        kv_ht_unlock (ht);
        kv_recovery_unlock ();
        kv_txm_unlock ();

        if (tx_id_list != NULL)
            free (tx_id_list);
        // add check log
        // unlock
    }
    pthread_mutex_unlock (lock);

    printf ("snapshot terminated\n");
    return NULL;
}
