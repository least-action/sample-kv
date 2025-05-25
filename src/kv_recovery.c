#include "kv_recovery.h"
#include "transaction.h"

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <pthread.h>

#define RECOVERY_FILE_NAME "recovery.kvdb"
#define MAX_LINE_LEN 128
#define LSN_ID_HEX_LEN 8

FILE *recovery_file;
static uint32_t lsn;
static pthread_mutex_t lsn_lock;

static const char BEGIN_TYPE[]      = "BEGIN__";
static const char COMMIT_TYPE[]     = "COMMIT_";
static const char UPDATE_TYPE[]     = "UPDATE_";
static const char ABORT_TYPE[]      = "ABORT__";
static const char CLR_TYPE[]        = "CLR____";
static const char CHECK_TYPE[]      = "CHECK__";
static const char END_TYPE[]        = "END____";


int kv_recovery_recover ()
{
    FILE *rec_file;
    // fpos_t pos;

    rec_file = fopen (RECOVERY_FILE_NAME, "w+");
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

    lsn = 0;  // todo: get from file
    pthread_mutex_init (&lsn_lock, NULL);

    return 0;
}

int kv_recovery_destroy ()
{
    pthread_mutex_destroy (&lsn_lock);
    fclose (recovery_file);
    return 0;
}

int kv_recovery_begin_log (lsn_id prev_id, uint32_t tx_id)
{
    uint32_t new_lsn;
    char line[MAX_LINE_LEN];
    
    pthread_mutex_lock (&lsn_lock);
    {
        new_lsn = ++lsn;
        snprintf (
            line, MAX_LINE_LEN,
            "%08x %08x T%08x %s\n",
            new_lsn, prev_id, tx_id, BEGIN_TYPE
        );  // todo: perf: use more efficient (ex. memset)
        
        if (fputs (line, recovery_file) == EOF) {
            // todo: error handling
        }
    }
    pthread_mutex_unlock (&lsn_lock);

    return new_lsn;
}

int kv_recovery_commit_log (lsn_id prev_id, uint32_t tx_id)
{
    uint32_t new_lsn;
    char line[MAX_LINE_LEN];
    
    pthread_mutex_lock (&lsn_lock);
    {
        new_lsn = ++lsn;
        snprintf (
            line, MAX_LINE_LEN,
            "%08x %08x T%08x %s\n",
            new_lsn, prev_id, tx_id, COMMIT_TYPE
        );  // todo: perf: use more efficient (ex. memset)
        
        if (fputs (line, recovery_file) == EOF) {
            // todo: error handling
        }
    }
    pthread_mutex_unlock (&lsn_lock);

    return new_lsn;
}

int kv_recovery_update_log (lsn_id prev_id, uint32_t tx_id)
{
    uint32_t new_lsn;
    char line[MAX_LINE_LEN];
    
    pthread_mutex_lock (&lsn_lock);
    {
        new_lsn = ++lsn;
        snprintf (
            line, MAX_LINE_LEN,
            "%08x %08x T%08x %s\n",
            new_lsn, prev_id, tx_id, UPDATE_TYPE
        );  // todo: perf: use more efficient (ex. memset)
        
        if (fputs (line, recovery_file) == EOF) {
            // todo: error handling
        }
    }
    pthread_mutex_unlock (&lsn_lock);

    return new_lsn;
}

int kv_recovery_abort_log (lsn_id prev_id, uint32_t tx_id)
{
    uint32_t new_lsn;
    char line[MAX_LINE_LEN];
    
    pthread_mutex_lock (&lsn_lock);
    {
        new_lsn = ++lsn;
        snprintf (
            line, MAX_LINE_LEN,
            "%08x %08x T%08x %s\n",
            new_lsn, prev_id, tx_id, ABORT_TYPE
        );  // todo: perf: use more efficient (ex. memset)
        
        if (fputs (line, recovery_file) == EOF) {
            // todo: error handling
        }
    }
    pthread_mutex_unlock (&lsn_lock);

    return new_lsn;
}

int kv_recovery_clr_log (lsn_id prev_id, uint32_t tx_id)
{
    uint32_t new_lsn;
    char line[MAX_LINE_LEN];
    
    pthread_mutex_lock (&lsn_lock);
    {
        new_lsn = ++lsn;
        snprintf (
            line, MAX_LINE_LEN,
            "%08x %08x T%08x %s\n",
            new_lsn, prev_id, tx_id, CLR_TYPE
        );  // todo: perf: use more efficient (ex. memset)
        
        if (fputs (line, recovery_file) == EOF) {
            // todo: error handling
        }
    }
    pthread_mutex_unlock (&lsn_lock);

    return new_lsn;
}

int kv_recovery_check_log (lsn_id prev_id, uint32_t tx_id)
{
    uint32_t new_lsn;
    char line[MAX_LINE_LEN];
    
    pthread_mutex_lock (&lsn_lock);
    {
        new_lsn = ++lsn;
        snprintf (
            line, MAX_LINE_LEN,
            "%08x %08x T%08x %s\n",
            new_lsn, prev_id, tx_id, CHECK_TYPE
        );  // todo: perf: use more efficient (ex. memset)
        
        if (fputs (line, recovery_file) == EOF) {
            // todo: error handling
        }
    }
    pthread_mutex_unlock (&lsn_lock);

    return new_lsn;
}

int kv_recovery_end_log (lsn_id prev_id, uint32_t tx_id)
{
    uint32_t new_lsn;
    char line[MAX_LINE_LEN];
    
    pthread_mutex_lock (&lsn_lock);
    {
        new_lsn = ++lsn;
        snprintf (
            line, MAX_LINE_LEN,
            "%08x %08x T%08x %s\n",
            new_lsn, prev_id, tx_id, END_TYPE
        );  // todo: perf: use more efficient (ex. memset)
        
        if (fputs (line, recovery_file) == EOF) {
            // todo: error handling
        }
    }
    pthread_mutex_unlock (&lsn_lock);

    return new_lsn;
}
