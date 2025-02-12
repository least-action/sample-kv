#include "kv_redoundo.h"
#include "kv_hash.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define REDO_LOG_FILE_NAME "redo_log.kvdb"
#define LOG_LINE_BUF_SIZE 1024

/*
 *  file format
 *
 *
 *  00000000 S 0005 0007 hello world!!\r\n
 *  00000001 D 0006 0000 hello\r\n
 */

int redo_log_write_fd;
int redo_log_read_fd;
char log_line_buf[LOG_LINE_BUF_SIZE];
int redo_id;

void kv_redo_init (void)
{
    redo_log_write_fd = open (REDO_LOG_FILE_NAME, O_CREAT | O_APPEND | O_WRONLY, 0644);
    if (redo_log_write_fd == -1) {
        perror ("open redo log write");
        exit (1);
    }
    redo_log_read_fd = open (REDO_LOG_FILE_NAME, O_RDONLY, 0644);
    if (redo_log_read_fd == -1) {
        perror ("open redo log read");
        exit (1);
    }

    redo_id = 0;
}

void kv_redo_terminate (void)
{
    close (redo_log_write_fd);
    close (redo_log_read_fd);
}

void int_to_digit (int digit_len, int integer, char* buf)
{
    int quotient, remainder;
    quotient = integer;
    for (int i = 0; i < digit_len; ++i) {
        remainder = quotient % 10;
        quotient /= 10;
        buf[digit_len-1-i] = (char) (remainder + 48);
    }
}

int get_next_redo_id (void)
{
    return redo_id++;
}

void kv_redo_add (enum RedoType redo_type, char *key, char *value)
{
    ssize_t nr;
    int key_len = strlen (key);
    int val_len = value == NULL ? 0 : strlen (value);
    int cur = 0;
    char id_digit[8];
    char key_digit[4];
    char val_digit[4];
    int_to_digit (4, key_len, key_digit);
    int_to_digit (4, val_len, val_digit);

    if (key_len + val_len > LOG_LINE_BUF_SIZE - 23) {
        perror ("too much long key and value");
        exit (1);
    }

    int_to_digit (8, get_next_redo_id (), id_digit);
    memcpy (log_line_buf, id_digit, 8);
    cur += 8;
    memcpy (log_line_buf + cur, " ", 1);
    cur += 1;

    if (redo_type == REDO_SET)
        memcpy (log_line_buf + cur, "S ", 2);
    else
        memcpy (log_line_buf + cur, "D ", 2);
    cur += 2;

    memcpy (log_line_buf + cur, key_digit, 4);
    cur += 4;
    memcpy (log_line_buf + cur, " ", 1);
    cur += 1;

    memcpy (log_line_buf + cur, val_digit, 4);
    cur += 4;
    memcpy (log_line_buf + cur, " ", 1);
    cur += 1;
 

    if (redo_type == REDO_SET) {
        memcpy (log_line_buf + cur, key, key_len);
        cur += key_len;
        memcpy (log_line_buf + cur, " ", 1);
        cur += 1;

        memcpy (log_line_buf + cur, value, val_len);
        cur += val_len;
    }
    else {
        memcpy (log_line_buf + cur, key, key_len);
        cur += key_len;
    }

    memcpy (log_line_buf + cur, "\r\n", 2);
    cur += 2;
    log_line_buf[cur] = '\0';

    nr = write (redo_log_write_fd, log_line_buf, strlen (log_line_buf) + 1);
    if (nr == -1) {
        perror ("write redo error");
        exit (1);
    }
}

void kv_redo_redo (struct kv_ht *ht)
{
    // todo
    char *key;
    char *value;
    ssize_t cur = 0;
    ssize_t nr;

    char number[8];
    char command;
    off_t ret;

    while (1) {
        break;
        nr = read (redo_log_read_fd, number, 8);
        if (nr == 0)
            break;
        if (nr != 8) {
            perror ("redo log file format error");
            exit (1);
        }

        ret = lseek (redo_log_read_fd, (off_t) 9, SEEK_SET);
        if (ret == (off_t) -1) {
            perror ("leek error");
            exit (1);
        }

        kv_ht_set (ht, key, value);
        kv_ht_del (ht, key);
    }

}

