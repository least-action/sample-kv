#include "kv_redoundo.h"
#include "kv_hash.h"
#include "utils.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>

// #define REDO_LOG_FILE_NAME "redo_log.kvdb"
#define RU_LOG_FILE_NAME "ru_log.kvdb"
#define LOG_LINE_BUF_SIZE 1024
#define ID_DIGIT_LEN 8
#define KEY_DIGIT_LEN 2
#define VAL_DIGIT_LEN 2
#define LINE_DIGIT_LEN 4

/*
 *  file format
 *
 *  00000001 T00000001 B
 *  00000002 T00000001 W 03 05 00 key old_value NULL
 *  00000003 T00000001 C
 *  00000004 T00000002 B
 *  00000005 T00000002 D 03 00 05 key NULL old_value
 *  00000006 T00000003 D 03 00 00 name NULL NULL
 *  00000007 T00000002 A
 *  00000008 T00000003 B
 *  00000009 T00000003 W 04 07 00 name michale NULL
 *  00000010 T00000003 W 03 09 00 key old_value NULL
 *  00000011 T00000003 W 03 09 09 key new_value old_value
 */


static int ru_log_write_fd;
static int ru_log_read_fd;
static int last_ru_id;
static pthread_mutex_t ru_lock;
char log_line_buf[LOG_LINE_BUF_SIZE];


ssize_t read_with_error (int fd, void *buf, size_t count)
{
    ssize_t nr;
    nr = read (fd, buf, count);
    if (nr == 0)
        return nr;
    if (nr != count) {
        perror ("redo log file format error");
        exit (1);
    }
    return nr;
}

off_t lseek_with_error (int fd, off_t offset, int whence)
{
    off_t ret;
    ret = lseek (fd, offset, whence);
    if (ret == (off_t) -1) {
        perror ("leek error");
        exit (1);
    }
    return ret;
}

void read_space (int fd)
{
    char space;
    read_with_error(fd, &space, 1);
    if (space != ' ') {
        perror ("redo log file format error (space)");
        exit (1);
    }
}

void read_eol (int fd)
{
    char eol;
    read_with_error(fd, &eol, 1);
    if (eol != '\n') {
        perror ("redo log file format error (eol)");
        exit (1);
    }
}

static int get_last_ru_id (int fd)
{
    char last_line_len_digit[LINE_DIGIT_LEN];
    int last_line_len;
    char last_ru_id_digit[ID_DIGIT_LEN];
    int last_ru_id;
    off_t position = lseek_with_error (fd, 0, SEEK_END);

    if (position == 0)
        return 0;

    lseek_with_error (fd, -LINE_DIGIT_LEN-1, SEEK_END);
    read_with_error (fd, last_line_len_digit, LINE_DIGIT_LEN);
    last_line_len = digit_to_int (last_line_len_digit, LINE_DIGIT_LEN);

    lseek_with_error (fd, -last_line_len, SEEK_END);
    read_with_error (fd, last_ru_id_digit, ID_DIGIT_LEN);
    last_ru_id = digit_to_int (last_ru_id_digit, ID_DIGIT_LEN);

    return last_ru_id;
}

void kv_ru_init ()
{
    // todo: fopen
    ru_log_write_fd = open (RU_LOG_FILE_NAME, O_CREAT | O_APPEND | O_WRONLY, 0644);
    if (ru_log_write_fd == -1) {
        perror ("open redo log write");
        exit (1);
    }
    ru_log_read_fd = open (RU_LOG_FILE_NAME, O_RDONLY, 0644);
    if (ru_log_read_fd == -1) {
        perror ("open redo log read");
        exit (1);
    }

    pthread_mutex_init (&ru_lock, NULL);

    last_ru_id = get_last_ru_id (ru_log_read_fd);
}

void kv_ru_destroy ()
{
    // todo: lock required?
    close (ru_log_write_fd);
    close (ru_log_read_fd);
}


void kv_ru_add (int tx_id, enum kv_ru_type ru_type, char *key, char *value, char *old_value)
{
    ssize_t nr;
    int key_len = key == NULL ? 0 : strlen (key);
    int val_len = value == NULL ? 0 : strlen (value);
    int old_len = old_value == NULL ? 0 : strlen (old_value);
    // todo: calculate once
    int line_len = (ID_DIGIT_LEN + 1) + (1 + ID_DIGIT_LEN + 1) + (1 + 1) +
                    (KEY_DIGIT_LEN + 1) + (VAL_DIGIT_LEN + 1) + (VAL_DIGIT_LEN + 1) +
                    (key_len + 1) + (val_len + 1) + (old_len + 1) +
                    LINE_DIGIT_LEN + 1;
    int cur = 0;
    char id_digit[ID_DIGIT_LEN];
    char tx_digit[ID_DIGIT_LEN];
    char key_len_digit[KEY_DIGIT_LEN];
    char val_len_digit[VAL_DIGIT_LEN];
    char old_len_digit[VAL_DIGIT_LEN];
    char line_len_digit[LINE_DIGIT_LEN];
    int_to_digit (ID_DIGIT_LEN, tx_id, tx_digit);
    int_to_digit (KEY_DIGIT_LEN, key_len, key_len_digit);
    int_to_digit (VAL_DIGIT_LEN, val_len, val_len_digit);
    int_to_digit (VAL_DIGIT_LEN, old_len, old_len_digit);
    int_to_digit (LINE_DIGIT_LEN, line_len, line_len_digit);

    if (key_len + val_len + old_len > LOG_LINE_BUF_SIZE - 50) {  // todo: re-calculate 50
        perror ("too much long key and value");
        exit (1);
    }

    cur = ID_DIGIT_LEN;
    memcpy (log_line_buf + cur, " ", 1);
    cur += 1;

    memcpy (log_line_buf + cur, "T", 1);
    cur += 1;
    memcpy (log_line_buf + cur, tx_digit, ID_DIGIT_LEN);
    cur += ID_DIGIT_LEN;
    memcpy (log_line_buf + cur, " ", 1);
    cur += 1;

    switch (ru_type) {
        case KV_RU_BEGIN:
            memcpy (log_line_buf + cur, "B ", 2);
            break;
        case KV_RU_ABORT:
            memcpy (log_line_buf + cur, "A ", 2);
            break;
        case KV_RU_COMMIT:
            memcpy (log_line_buf + cur, "C ", 2);
            break;
        case KV_RU_WRITE:
            memcpy (log_line_buf + cur, "W ", 2);
            break;
        case KV_RU_DELETE:
            memcpy (log_line_buf + cur, "D ", 2);
            break;
        default:
            perror ("ru_type not handled");
            exit (1);
            break;
    }
    cur += 2;

    {
        memcpy (log_line_buf + cur, key_len_digit, KEY_DIGIT_LEN);
        cur += KEY_DIGIT_LEN;
        memcpy (log_line_buf + cur, " ", 1);
        cur += 1;
        memcpy (log_line_buf + cur, val_len_digit, VAL_DIGIT_LEN);
        cur += VAL_DIGIT_LEN;
        memcpy (log_line_buf + cur, " ", 1);
        cur += 1;
        memcpy (log_line_buf + cur, old_len_digit, VAL_DIGIT_LEN);
        cur += VAL_DIGIT_LEN;
        memcpy (log_line_buf + cur, " ", 1);
        cur += 1;

        memcpy (log_line_buf + cur, key, key_len);
        cur += key_len;
        memcpy (log_line_buf + cur, " ", 1);
        cur += 1;
        memcpy (log_line_buf + cur, value, val_len);
        cur += val_len;
        memcpy (log_line_buf + cur, " ", 1);
        cur += 1;
        memcpy (log_line_buf + cur, old_value, old_len);
        cur += old_len;
        memcpy (log_line_buf + cur, " ", 1);
        cur += 1;
    }

    memcpy (log_line_buf + cur, line_len_digit, LINE_DIGIT_LEN);
    cur += LINE_DIGIT_LEN;
    
    memcpy (log_line_buf + cur, "\n", 1);
    cur += 1;
    log_line_buf[cur] = '\0';

    pthread_mutex_lock (&ru_lock);
    {
        int_to_digit (8, ++last_ru_id, id_digit);
        memcpy (log_line_buf, id_digit, 8);
        nr = write (ru_log_write_fd, log_line_buf, strlen (log_line_buf));
    }
    pthread_mutex_unlock (&ru_lock);

    if (nr == -1) {
        perror ("write redo error");
        exit (1);
    }
}

void kv_ru_redo (struct kv_ht *ht)
{
    // char number[8];
    // memset (number, '0', 8);
    // char command;
    // char key_len_digit[4];
    // int key_len;
    // char val_len_digit[4];
    // int val_len;
    // char key[1024];
    // char val[1024];

    // // todo: add log line to error string
    // while (1) {
    //     nr = read_with_error (redo_log_read_fd, number, 8);
    //     if (nr == 0) {
    //         redo_id = digit_to_int (number, 8) + 1;
    //         break;
    //     }
    //     read_space (redo_log_read_fd);

    //     nr = read_with_error (redo_log_read_fd, &command, 1);
    //     read_space (redo_log_read_fd);

    //     nr = read_with_error (redo_log_read_fd, key_len_digit, 4);
    //     // todo: key format check(\r\n existence)
    //     key_len = digit_to_int (key_len_digit, 4);
    //     read_space (redo_log_read_fd);

    //     nr = read_with_error (redo_log_read_fd, val_len_digit, 4);
    //     val_len = digit_to_int (val_len_digit, 4);
    //     read_space (redo_log_read_fd);


    //     if (command == 'S') {
    //         nr = read_with_error (redo_log_read_fd, key, key_len);
    //         key[key_len] = '\0';
    //         read_space (redo_log_read_fd);

    //         nr = read_with_error (redo_log_read_fd, val, val_len);
    //         val[val_len] = '\0';

    //         read_eol (redo_log_read_fd);

    //         kv_ht_set (ht, key, val);
    //     }
    //     else if (command == 'D') {
    //         nr = read_with_error (redo_log_read_fd, key, key_len);
    //         key[key_len] = '\0';

    //         read_eol (redo_log_read_fd);

    //         kv_ht_del (ht, key);
    //     }
    //     else {
    //         perror ("command error");
    //         exit (1);
    //     }
    // }
}

void kv_ru_undo (int tx_id)
{
    // todo
}