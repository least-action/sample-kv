#include "kv_client_handler.h"
#include "kv_server.h"
#include "kv_command.h"
#include "lock_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stddef.h>
#include <stdbool.h>


#define BUFFER_SIZE 128
#define COMMAND_SIZE 128
#define RESULT_SIZE 64


void* kv_handle_client (void *data)
{
    struct kv_handle_client_data *client_data;
    client_data = (struct kv_handle_client_data *) data;
    int client_fd = client_data->client_fd;
    struct kv_lm *lm = client_data->lm;
    free (client_data);

    char buffer[BUFFER_SIZE];
    /*
     * command
     * [s][e][t][ ][a][b][c][\0]
     *                        ^
     *                        idx(==7) = command_cur 
     */
    char command[COMMAND_SIZE];
    int command_cur = 0;
    int tx_id = 0;
    char result[RESULT_SIZE];
    size_t result_len;
    bool is_disconnected = false;
    int consume_count;

    while (!is_disconnected) {
        // todo: bug: when user send more than BUFFER_SIZE
        memset (buffer, 0, BUFFER_SIZE);
        int bytes_read = read (client_fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read <= 0) {
            // todo: end transaction
            close (client_fd);
            printf ("client disconnected\n");
            is_disconnected = true;
            break;
        }

        // todo: bug: handle when command is longer than limit
        memcpy (command + command_cur, buffer, bytes_read);
        command_cur += bytes_read;
        
        while (1) {
            consume_count = consume_command (ht, lm, command, result, &tx_id);
            if (consume_count == 0)
                break;
            command_cur = strlen (command);

            if (result[0] != '\0')
                strcpy (result + strlen(result), "\r\n");
            result_len = strlen (result) + 1;

            if (write (client_fd, result, result_len) < result_len) {
                perror ("failed to write");
                close (client_fd);
                printf ("client disconnected\n");
                is_disconnected = true;
                break;
            }
        }
    }
    return NULL;
}
