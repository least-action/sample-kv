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

const char* log_type_str[] = {
    "BEGIN__",
    "COMMIT_",
    "UPDATE_",
    "ABORT__",
    "CLR____",
    "CHECK__",
    "END____",
};

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

static int lsn_id_to_hex (uint32_t lsn_id, char* hex)
{
    if (LSN_ID_HEX_LEN != sizeof (lsn_id) * 2) {
        return 1;
    }

    uint32_t filter;
    uint32_t digit;    
    
    filter = 0x0000000f;

    for (int i = 0; i < LSN_ID_HEX_LEN; ++i) {
        digit = (lsn_id & filter) >> (i * 4);
        hex[LSN_ID_HEX_LEN - i - 1] = digit + 48;
        filter <<= 4;
    }

    return 0;
}

static int tx_id_to_hex (uint32_t tx_id, char* hex)
{
    return lsn_id_to_hex (tx_id, hex);
}

int kv_recovery_add_log (uint32_t prev_id, uint32_t tx_id, enum kv_recovery_log_type log_type)
{
    uint32_t new_lsn;
    // char lsn_hex[LSN_ID_HEX_LEN+1];
    // char prev_lsn_hex[LSN_ID_HEX_LEN+1];
    // char tx_id_hex[TX_ID_HEX_LEN+1];
    char line[MAX_LINE_LEN];
    
    // lsn_hex[LSN_ID_HEX_LEN] = '\0';
    // prev_lsn_hex[LSN_ID_HEX_LEN] = '\0';
    // tx_id_hex[TX_ID_HEX_LEN] = '\0';

    
    // lsn_id_to_hex (prev_id, prev_lsn_hex);
    // tx_id_to_hex (tx_id, tx_id_hex);

    // printf("%u %u %u\n", new_lsn, prev_id, tx_id);

    

    pthread_mutex_lock (&lsn_lock);
    {
        new_lsn = ++lsn;
        // lsn_id_to_hex (new_lsn, lsn_hex);
        snprintf (
            line, MAX_LINE_LEN,
            "%08x %08x T%08x %s\n",
            new_lsn, prev_id, tx_id, log_type_str[log_type]
        );
        
        if (fputs (line, recovery_file) == EOF) {
            // todo: error handling
        }
    }
    pthread_mutex_unlock (&lsn_lock);

    
    // todo
    return new_lsn;
}
