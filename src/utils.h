#ifndef __KV_UTILS_H__
#define __KV_UTILS_H__

#include <sys/types.h>
#include <unistd.h>

int digit_to_int (char* digit, int digit_len);
void int_to_digit (int digit_len, int integer, char* buf);

off_t lseek_with_error (int fd, off_t offset, int whence);
ssize_t read_with_error (int fd, void *buf, size_t count);

#endif
