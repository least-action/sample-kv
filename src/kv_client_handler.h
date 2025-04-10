#ifndef __KV_CLIENT_HANDLER_H__
#define __KV_CLIENT_HANDLER_H__

#include "lock_manager.h"

#define BUFFER_SIZE 128
#define COMMAND_SIZE 128
#define RESULT_SIZE 64

struct kv_handle_client_data {
    int client_fd;
    struct kv_lm *lm;
};

void* kv_handle_client (void *data);

#endif