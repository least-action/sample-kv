#include "kv_recovery.h"
#include "transaction.h"
#include "utils.h"

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

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
        hex[7 - i] = get_hex (digit);
        filter <<= 4;
    }

    return 0;
}

static uint32_t hex_to_uint32 (char *hex)
{
    uint32_t hex_val;

    hex_val = 0;
    for (int i = 0; i < 8; ++i) {
        hex_val += from_hex (hex[i]) << (7-i);
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
        hex[1 - i] = get_hex (digit);
        filter <<= 4;
    }

    return 0;
}

static size_t from_2_digit_hex (char *hex)
{
    size_t hex_val;

    hex_val = 0;
    for (int i = 0; i < 2; ++i) {
        hex_val += from_hex (hex[i]) << (1-i);
    }

    return hex_val;
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
            "%08X %08X T%08X %s\n",
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
            "%08X %08X T%08X %s\n",
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

static int data_change_log (lsn_id prev_id, uint32_t tx_id, char *key, size_t key_len, char *old_val, size_t old_len, char *new_val, size_t new_len, bool is_clr)
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

    if (is_clr) {
        memcpy (pos, CLR_TYPE, 7);
        pos += 7;
    } else {
        memcpy (pos, UPDATE_TYPE, 7);
        pos += 7;
    }
    
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
    
    memcpy (pos, "\n", 1);
    pos += 1;
    memcpy (pos, "\0", 1);

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

int kv_recovery_update_log (lsn_id prev_id, uint32_t tx_id, char *key, size_t key_len, char *old_val, size_t old_len, char *new_val, size_t new_len)
{
    return data_change_log (prev_id, tx_id, key, key_len, old_val, old_len, new_val, new_len, false);
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
            "%08X %08X T%08X %s\n",
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
    return data_change_log (prev_id, tx_id, key, key_len, cur_val, cur_len, prev_val, prev_len, true);
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
            "%08X %08X T%08X %s\n",
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
            "%08X %08X T%08X %s\n",
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
    struct kv_recovery_log_line *log_line;
    FILE *recovery_file;
    char line[KV_RECOVERY_MAX_LINE_LEN];
    char lsn_hex[8];
    uint32_t line_lsn;
    uint32_t prev_lsn;
    uint32_t tx_id;
    char log_type_char[LOG_TYPE_LEN];
    enum kv_recovery_log_type log_type;

    char len_hex[2];
    char *key;
    size_t key_len;
    char *old_val;
    size_t old_len;
    char *new_val;
    size_t new_len;

    size_t pos;
    
    recovery_file = fopen (RECOVERY_FILE_NAME, "r");  // todo: perf: reuse file object
    
    line_lsn = -1;
    while ((fgets (line, KV_RECOVERY_MAX_LINE_LEN, recovery_file)) != NULL) {  // todo: perf
        memcpy (lsn_hex, line, 8);
        line_lsn = hex_to_uint32 (lsn_hex);
        
        if (line_lsn == lsn)
            break;
    }
    if (line_lsn != lsn) {
        // todo: error handling
        fclose (recovery_file);
        return NULL;
    }
    
    pos = 9;
    memcpy (lsn_hex, line + pos, 8);
    pos += 8 + 1;
    prev_lsn = hex_to_uint32 (lsn_hex);

    pos += 1;
    memcpy (lsn_hex, line + pos, 8);
    pos += 8 + 1;
    tx_id = hex_to_uint32 (lsn_hex);
    memcpy (log_type_char, line + pos, LOG_TYPE_LEN);
    pos += LOG_TYPE_LEN + 1;
    
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
        memcpy (len_hex, line + pos, 2);
        pos += 2 + 1;
        key_len = from_2_digit_hex (len_hex);
        memcpy (len_hex, line + pos, 2);
        pos += 2 + 1;
        old_len = from_2_digit_hex (len_hex);
        memcpy (len_hex, line + pos, 2);
        pos += 2 + 1;
        new_len = from_2_digit_hex (len_hex);
        key = malloc (key_len);
        old_val = malloc (old_len);
        new_val = malloc (new_len);
        memcpy (key, line + pos, key_len);
        pos += key_len + 1;
        memcpy (old_val, line + pos, old_len);
        pos += old_len + 1;
        memcpy (new_val, line + pos, new_len);
    } else {
        key = NULL;
        key_len = 0;
        old_val = NULL;
        old_len = 0;
        new_val = NULL;
        new_len = 0;
    }
    
    log_line = (struct kv_recovery_log_line *) malloc (sizeof (struct kv_recovery_log_line));
    log_line->lsn = lsn;
    log_line->prev_lsn = prev_lsn;
    log_line->tx_id = tx_id;
    log_line->log_type = log_type;
    log_line->key = key;
    log_line->key_len = key_len;
    log_line->old_val = old_val;
    log_line->old_len = old_len;
    log_line->new_val = new_val;
    log_line->new_len = new_len;

    fclose (recovery_file);

    return log_line;
}

int kv_recovery_destroy_log_line (struct kv_recovery_log_line *log)
{
    if (log == NULL)
        return 0;
    
    free (log->key);
    free (log->old_val);
    free (log->new_val);
    free (log);
    
    return 0;
}
