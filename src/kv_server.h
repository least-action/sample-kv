#ifndef __KV_SERVER_H__
#define __KV_SERVER_H__

#include <stdint.h>

#define MAX_EVENTS 64

extern int epfd;
extern int server_fd;
extern struct kv_ht *ht;

int kv_run_server (uint16_t port);

#endif
