#include "kv_recovery_snapshot.h"
#include "shutdown.h"
#include "kv_recovery.h"
#include "tx_manager.h"
#include "kv_hash.h"

#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <wait.h>
#include <unistd.h>
#include <sys/stat.h>

#define SNAPSHOT_DIR_NAME "snapshots"
#define SNAPSHOT_FILE_PREFIX "snapshot"
#define SNAPSHOT_FILE_EXT "kvdb"

static const time_t DELAY = 5;

// static kv_hashtable_foreach ()
// {
//     return 
// }

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
    pid_t pid;
    int child_status;
    uint32_t check_lsn;
    FILE *snapshot_file;
    time_t epoch_time;
    char snapshot_file_name[100];
    char line_buffer[128];

    pid = 0;
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
            check_lsn = kv_recovery_check_log (tx_id_list, tx_id_list_size);

            pid = fork ();
            if (pid != 0) {  // parent
                // pass
            } else {  // child
                is_running = false;
            }
        }
        kv_ht_unlock (ht);
        kv_recovery_unlock ();
        kv_txm_unlock ();
        
        if (tx_id_list != NULL)
            free (tx_id_list);
        
        wait (&child_status);  // todo: handling
    }
    pthread_mutex_unlock (lock);

    if (pid != 0) {
        printf ("snapshot terminated\n");
        return NULL;
    }

    // snapshot process

    // create file
    // write file
    // close file
    // update recent file
    epoch_time = time (NULL);
    snprintf (
        snapshot_file_name, sizeof (snapshot_file_name),
        "%s/%s_%ld.%s",
        SNAPSHOT_DIR_NAME, SNAPSHOT_FILE_PREFIX, epoch_time, SNAPSHOT_FILE_EXT
    );

    if (mkdir (SNAPSHOT_DIR_NAME, 0755) == -1) {
        if (errno != EEXIST) {
            exit(1);
        }
    }
    snapshot_file = fopen (snapshot_file_name, "w+");
    snprintf (line_buffer, sizeof (line_buffer), "%08X\n", check_lsn);
    fputs (line_buffer, snapshot_file);
    fclose (snapshot_file);
    exit (0);
}
