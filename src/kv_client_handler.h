#ifndef __KV_CLIENT_HANDLER_H__
#define __KV_CLIENT_HANDLER_H__

#define BUFFER_SIZE 128
#define COMMAND_SIZE 128
#define RESULT_SIZE 64


struct handler_context {
    int client_fd;
    char buffer[BUFFER_SIZE];
    /*
     * command
     * [s][e][t][ ][a][b][c][\0]
     *                        ^
     *                        idx(==7) = command_cur 
     */
    char command[COMMAND_SIZE];
    int command_cur;
    int tx_id;
    char result[RESULT_SIZE];
    struct handler_context *prev;  // todo: perf: optimize O(n)
    struct handler_context *next;
};


void register_client (int client_fd);
void remove_client (int client_fd);
void handle_client (int client_fd);

#endif