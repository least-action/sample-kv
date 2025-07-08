#include "kv_recovery_snapshot.h"
#include "shutdown.h"
#include "kv_recovery.h"
#include "tx_manager.h"
#include "kv_hash.h"
#include "kv_command.h"

#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <wait.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

#define SNAPSHOT_DIR_NAME "snapshots"
#define SNAPSHOT_FILE_PREFIX "snapshot"
#define SNAPSHOT_FILE_EXT "kvdb"

static const time_t DELAY = 5;

static void kv_hashtable_foreach (const struct kv_ht_kv kv, void *param)
{
    FILE *snapshot_file;
    snapshot_file = (FILE *) param;
    char line_buff[128];
    struct key_data *kd;
    struct val_data *vd;
    int cur;
    
    kd = kv.key;
    vd = kv.value;
    cur = 0;

    memcpy (line_buff + cur, kd->key, kd->key_len);
    cur += kd->key_len;
    memcpy (line_buff + cur, " " , 1);
    cur += 1;
    memcpy (line_buff + cur, vd->value, vd->val_len);
    cur += vd->val_len;
    memcpy (line_buff + cur, "\n\0", 2);

    fputs (line_buff, snapshot_file);

    return;
}

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
    char snapshot_file_name[100];

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
    // 1. create file
    snprintf (
        snapshot_file_name, sizeof (snapshot_file_name),
        "%s/%s_%08X.%s",
        SNAPSHOT_DIR_NAME, SNAPSHOT_FILE_PREFIX, check_lsn, SNAPSHOT_FILE_EXT
    );

    if (mkdir (SNAPSHOT_DIR_NAME, 0755) == -1) {
        if (errno != EEXIST) {
            exit(1);
        }
    }
    snapshot_file = fopen (snapshot_file_name, "w+");
    
    // 2. write file
    kv_ht_foreach (ht, kv_hashtable_foreach, snapshot_file);

    // 3. close file
    fclose (snapshot_file);

    exit (0);
}
