#include "utils.h"

#include <stdlib.h>
#include <stdio.h>

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
