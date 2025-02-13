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
 *  00000000 S 0005 0007 hello world!!\n
 *  00000001 D 0006 0000 hello\n
 */

int redo_log_write_fd;
int redo_log_read_fd;
char log_line_buf[LOG_LINE_BUF_SIZE];
int redo_id;

void kv_redo_init (void)
{
    // todo: fopen
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

    // todo: get final redo_id
    redo_id = 0;
}

void kv_redo_terminate (void)
{
    close (redo_log_write_fd);
    close (redo_log_read_fd);
}

// todo: perf: to hex?
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

int digit_to_int (char* digit, int digit_len)
{
    int integer = 0;
    int weight = 1;
    int num;
    for (int i = 0; i < digit_len; ++i) {
        num = (int) (digit[digit_len-1-i] - 48);
        if (num < 0 || num > 9) {
            perror ("digit is out of range");
            exit (1);
        }
        integer += num * weight;
        weight *= 10;
    }
    return integer;
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

    memcpy (log_line_buf + cur, "\n", 1);
    cur += 1;
    log_line_buf[cur] = '\0';

    nr = write (redo_log_write_fd, log_line_buf, strlen (log_line_buf));
    if (nr == -1) {
        perror ("write redo error");
        exit (1);
    }
}


void lseek_with_error (int fd, off_t cur, int whence)
{
    off_t ret;
    ret = lseek (fd, cur, whence);
    if (ret == (off_t) -1) {
        perror ("leek error");
        exit (1);
    }
    return;
}

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

void kv_redo_redo (struct kv_ht *ht)
{
    ssize_t nr;

    char number[8];
    char command;
    char key_len_digit[4];
    int key_len;
    char val_len_digit[4];
    int val_len;
    char key[1024];
    char val[1024];

    // todo: add log line to error string
    while (1) {
        nr = read_with_error (redo_log_read_fd, number, 8);
        if (nr == 0)
            break;
        read_space (redo_log_read_fd);

        nr = read_with_error (redo_log_read_fd, &command, 1);
        read_space (redo_log_read_fd);

        nr = read_with_error (redo_log_read_fd, key_len_digit, 4);
        // todo: key format check(\r\n existence)
        key_len = digit_to_int (key_len_digit, 4);
        read_space (redo_log_read_fd);

        nr = read_with_error (redo_log_read_fd, val_len_digit, 4);
        val_len = digit_to_int (val_len_digit, 4);
        read_space (redo_log_read_fd);


        if (command == 'S') {
            nr = read_with_error (redo_log_read_fd, key, key_len);
            key[key_len] = '\0';
            read_space (redo_log_read_fd);

            nr = read_with_error (redo_log_read_fd, val, val_len);
            val[val_len] = '\0';

            read_eol (redo_log_read_fd);

            kv_ht_set (ht, key, val);
        }
        else if (command == 'D') {
            nr = read_with_error (redo_log_read_fd, key, key_len);
            key[key_len] = '\0';

            read_eol (redo_log_read_fd);

            kv_ht_del (ht, key);
        }
        else {
            perror ("command error");
            exit (1);
        }
    }
}

