#include "kv_recovery.h"
#include "transaction.h"

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

#define RECOVERY_FILE_NAME "recovery.kvdb"
#define LSN_ID_HEX_LEN 8
#define LOG_TYPE_LEN 7

FILE *recovery_file;
static uint32_t lsn;
static pthread_mutex_t lsn_lock;

static const char BEGIN_TYPE[LOG_TYPE_LEN+1]      = "BEGIN__";
static const char COMMIT_TYPE[LOG_TYPE_LEN+1]     = "COMMIT_";
static const char UPDATE_TYPE[LOG_TYPE_LEN+1]     = "UPDATE_";
static const char ABORT_TYPE[LOG_TYPE_LEN+1]      = "ABORT__";
static const char CLR_TYPE[LOG_TYPE_LEN+1]        = "CLR____";
static const char CHECK_TYPE[LOG_TYPE_LEN+1]      = "CHECK__";
static const char END_TYPE[LOG_TYPE_LEN+1]        = "END____";


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

static int uint32_to_hex (uint32_t lsn_id, char* hex)
{
    uint32_t filter;
    uint32_t digit;    
    
    filter = 0x0000000f;
    for (int i = 0; i < 8; ++i) {
        digit = (lsn_id & filter) >> (i * 4);
        hex[7 - i] = digit + 48;
        filter <<= 4;
    }

    return 0;
}

static uint32_t hex_to_uint32 (char *hex)
{
    uint32_t hex_val;

    hex_val = 0;
    for (int i = 0; i < 8; ++i) {
        hex_val += (hex[i] - 48) << (7-i);
    }

    return hex_val;
}

static int size_t_to_2_digit_hex (size_t s, char *hex)
{
    uint32_t filter;
    uint32_t digit;    
    
    filter = 0x0000000f;
    for (int i = 0; i < 2; ++i) {
        digit = (s & filter) >> (i * 4);
        hex[1 - i] = digit + 48;
        filter <<= 4;
    }

    return 0;
}

int kv_recovery_begin_log (lsn_id prev_id, uint32_t tx_id)
{
    uint32_t new_lsn;
    char line[KV_RECOVERY_MAX_LINE_LEN];
    
    pthread_mutex_lock (&lsn_lock);
    {
        new_lsn = ++lsn;
        snprintf (
            line, KV_RECOVERY_MAX_LINE_LEN,
            "%08x %08x T%08x %s\n",
            new_lsn, prev_id, tx_id, BEGIN_TYPE
        );  // todo: perf: use more efficient (ex. memset)
        
        if (fputs (line, recovery_file) == EOF) {
            // todo: error handling
        }
        fflush (recovery_file);
    }
    pthread_mutex_unlock (&lsn_lock);

    return new_lsn;
}

int kv_recovery_commit_log (lsn_id prev_id, uint32_t tx_id)
{
    uint32_t new_lsn;
    char line[KV_RECOVERY_MAX_LINE_LEN];
    
    pthread_mutex_lock (&lsn_lock);
    {
        new_lsn = ++lsn;
        snprintf (
            line, KV_RECOVERY_MAX_LINE_LEN,
            "%08x %08x T%08x %s\n",
            new_lsn, prev_id, tx_id, COMMIT_TYPE
        );  // todo: perf: use more efficient (ex. memset)
        
        if (fputs (line, recovery_file) == EOF) {
            // todo: error handling
        }
        fflush (recovery_file);
    }
    pthread_mutex_unlock (&lsn_lock);

    return new_lsn;
}

int kv_recovery_update_log (lsn_id prev_id, uint32_t tx_id, char *key, size_t key_len, char *old_val, size_t old_len, char *new_val, size_t new_len)
{
    uint32_t new_lsn;
    char line[KV_RECOVERY_MAX_LINE_LEN];
    char uint32_hex[8];
    char size_t_hex[2];
    char *pos;

    pos = line;

    pos += 8;

    memcpy (pos, " ", 1);
    pos += 1;

    uint32_to_hex (prev_id, uint32_hex);
    memcpy (pos, uint32_hex, 8);
    pos += 8;
    
    memcpy (pos, " ", 1);
    pos += 1;
    
    memcpy (pos, "T", 1);
    pos += 1;
    uint32_to_hex (tx_id, uint32_hex);
    memcpy (pos, uint32_hex, 8);
    pos += 8;

    memcpy (pos, " ", 1);
    pos += 1;

    memcpy (pos, UPDATE_TYPE, 7);
    pos += 7;

    memcpy (pos, " ", 1);
    pos += 1;

    size_t_to_2_digit_hex (key_len, size_t_hex);
    memcpy (pos, size_t_hex, 2);
    pos += 2;
    memcpy (pos, " ", 1);
    pos += 1;

    size_t_to_2_digit_hex (old_len, size_t_hex);
    memcpy (pos, size_t_hex, 2);
    pos += 2;
    memcpy (pos, " ", 1);
    pos += 1;

    size_t_to_2_digit_hex (new_len, size_t_hex);
    memcpy (pos, size_t_hex, 2);
    pos += 2;
    memcpy (pos, " ", 1);
    pos += 1;

    memcpy (pos, key, key_len);
    pos += key_len;
    memcpy (pos, " ", 1);
    pos += 1;

    memcpy (pos, old_val, old_len);
    pos += old_len;
    memcpy (pos, " ", 1);
    pos += 1;

    memcpy (pos, new_val, new_len);
    pos += new_len;
    memcpy (pos, " ", 1);
    pos += 1;

    memcpy (pos, "\n", 1);

    pos = line;
    pthread_mutex_lock (&lsn_lock);
    {
        new_lsn = ++lsn;
        uint32_to_hex (new_lsn, uint32_hex);
        memcpy (pos, uint32_hex, 8);
        
        if (fputs (line, recovery_file) == EOF) {
            // todo: error handling
        }
        fflush (recovery_file);
    }
    pthread_mutex_unlock (&lsn_lock);

    return new_lsn;
}

int kv_recovery_abort_log (lsn_id prev_id, uint32_t tx_id)
{
    uint32_t new_lsn;
    char line[KV_RECOVERY_MAX_LINE_LEN];
    
    pthread_mutex_lock (&lsn_lock);
    {
        new_lsn = ++lsn;
        snprintf (
            line, KV_RECOVERY_MAX_LINE_LEN,
            "%08x %08x T%08x %s\n",
            new_lsn, prev_id, tx_id, ABORT_TYPE
        );  // todo: perf: use more efficient (ex. memset)
        
        if (fputs (line, recovery_file) == EOF) {
            // todo: error handling
        }
        fflush (recovery_file);
    }
    pthread_mutex_unlock (&lsn_lock);

    return new_lsn;
}

int kv_recovery_clr_log (lsn_id prev_id, uint32_t tx_id, uint32_t undo_next_lsn, char *key, size_t key_len, char *cur_val, size_t cur_len, char *prev_val, size_t prev_len)
{
    uint32_t new_lsn;
    char line[KV_RECOVERY_MAX_LINE_LEN];
    
    pthread_mutex_lock (&lsn_lock);
    {
        new_lsn = ++lsn;
        snprintf (
            line, KV_RECOVERY_MAX_LINE_LEN,
            "%08x %08x T%08x %s %08x\n",
            new_lsn, prev_id, tx_id, CLR_TYPE, undo_next_lsn
        );  // todo: perf: use more efficient (ex. memset)
        
        if (fputs (line, recovery_file) == EOF) {
            // todo: error handling
        }
        fflush (recovery_file);
    }
    pthread_mutex_unlock (&lsn_lock);

    return new_lsn;
}

int kv_recovery_check_log (lsn_id prev_id, uint32_t tx_id)
{
    uint32_t new_lsn;
    char line[KV_RECOVERY_MAX_LINE_LEN];
    
    pthread_mutex_lock (&lsn_lock);
    {
        new_lsn = ++lsn;
        snprintf (
            line, KV_RECOVERY_MAX_LINE_LEN,
            "%08x %08x T%08x %s\n",
            new_lsn, prev_id, tx_id, CHECK_TYPE
        );  // todo: perf: use more efficient (ex. memset)
        
        if (fputs (line, recovery_file) == EOF) {
            // todo: error handling
        }
        fflush (recovery_file);
    }
    pthread_mutex_unlock (&lsn_lock);

    return new_lsn;
}

int kv_recovery_end_log (lsn_id prev_id, uint32_t tx_id)
{
    uint32_t new_lsn;
    char line[KV_RECOVERY_MAX_LINE_LEN];
    
    pthread_mutex_lock (&lsn_lock);
    {
        new_lsn = ++lsn;
        snprintf (
            line, KV_RECOVERY_MAX_LINE_LEN,
            "%08x %08x T%08x %s\n",
            new_lsn, prev_id, tx_id, END_TYPE
        );  // todo: perf: use more efficient (ex. memset)
        
        if (fputs (line, recovery_file) == EOF) {
            // todo: error handling
        }
        fflush (recovery_file);
    }
    pthread_mutex_unlock (&lsn_lock);

    return new_lsn;
}

struct kv_recovery_log_line* kv_recovery_get_log (lsn_id lsn)
{
    // todo
    struct kv_recovery_log_line *log_line;
    FILE *recovery_file;
    char line[KV_RECOVERY_MAX_LINE_LEN];
    char lsn_hex[8];
    uint32_t line_lsn;
    uint32_t prev_lsn;
    uint32_t tx_id;
    char log_type_char[LOG_TYPE_LEN];
    enum kv_recovery_log_type log_type;
    char *key;
    size_t key_len;
    char *old_val;
    size_t old_len;
    char *new_val;
    size_t new_len;
    
    recovery_file = fopen (RECOVERY_FILE_NAME, "r");  // todo: perf: reuse file object

    log_line = NULL;
    while ((fgets (line, KV_RECOVERY_MAX_LINE_LEN, recovery_file)) != NULL) {
        memcpy (lsn_hex, line, 8);
        line_lsn = hex_to_uint32 (lsn_hex);
        if (line_lsn != lsn)
            continue;
        
        memcpy (lsn_hex, line + 9, 8);
        prev_lsn = hex_to_uint32 (lsn_hex);

        memcpy (lsn_hex, line + 19, 8);
        tx_id = hex_to_uint32 (lsn_hex);

        memcpy (log_type_char, line + 28, LOG_TYPE_LEN);
        if (log_type_char[0] == 'B') {
            log_type = KV_REC_BEGIN;
        } else if (log_type_char[0] == 'C') {
            if (log_type_char[1] == 'O') {
                log_type = KV_REC_COMMIT;
            } else if (log_type_char[1] == 'L') {
                log_type = KV_REC_CLR;
            } else {
                log_type = KV_REC_CHECK;
            }
        } else if (log_type_char[0] == 'U') {
            log_type = KV_REC_UPDATE;
        } else if (log_type_char[0] == 'A') {
            log_type = KV_REC_ABORT;
        } else {
            log_type = KV_REC_END;
        }

        if (log_type_char[0] == 'U') {
            // todo
        }

        log_line = (struct kv_recovery_log_line *) malloc (sizeof (struct kv_recovery_log_line));
        log_line->lsn = line_lsn;
        log_line->prev_lsn = prev_lsn;
        log_line->tx_id = tx_id;
        log_line->log_type = log_type;
        log_line->key = NULL;
        log_line->key_len = 0;
        log_line->old_val = NULL;
        log_line->old_len = 0;
        log_line->new_val = NULL;
        log_line->new_len = 0;
        
        break;
    }

    fclose (recovery_file);

    return log_line;
}

int kv_recovery_destroy_log_line (struct kv_recovery_log_line *log)
{
    if (log == NULL)
        return 0;
    
    // todo
    free (log);
    return 0;
}
