#ifndef __KV_CLIENT_HANDLER_H__
#define __KV_CLIENT_HANDLER_H__

#define BUFFER_SIZE 128
#define COMMAND_SIZE 128
#define RESULT_SIZE 64

void kv_handle_client (int client_fd);

#endif