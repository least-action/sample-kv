#include "kv_recovery.h"

#include <stdio.h>

#define RECOVERY_FILE_NAME "recovery.kvdb"

pthread_mutex_t kv_ru_lock;  // todo: init

FILE *recovery_file;

int kv_recovery_recover ()
{
    FILE *rec_file;
    // fpos_t pos;

    rec_file = fopen (RECOVERY_FILE_NAME, "r+");
    if (!rec_file) {
        // todo: error handling
    }

    // build hash table data from dump file

    // analysis
        // find check point
        // add unterminated tx list

    // redo
    

    // undo

    fclose (rec_file);

    // todo

    return 0;
}

int kv_recovery_init ()
{
    recovery_file = fopen (RECOVERY_FILE_NAME, "a");
    if (!recovery_file) {
        // todo: error handling
    }

    return 0;
}

int kv_recovery_destroy ()
{
    fclose (recovery_file);
    return 0;
}

int kv_recovery_add_log ()
{
    // todo
    return 0;
}
