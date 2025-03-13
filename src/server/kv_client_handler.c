#include "server/kv_client_handler.h"
#include "server/kv_server.h"
#include "kv_command.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <stddef.h>


struct handler_context *hc;

void register_client (int client_fd)
{
    struct handler_context *last;
    struct handler_context *new_context;
    new_context = (struct handler_context*) malloc (sizeof (struct handler_context));
    new_context->client_fd = client_fd;
    memset (new_context->buffer, 0, BUFFER_SIZE);
    memset (new_context->command, 0, COMMAND_SIZE);
    new_context->command_cur = 0;
    new_context->tx_id = -1;
    memset (new_context->result, 0, RESULT_SIZE);
    new_context->prev = NULL;
    new_context->next = NULL;
    if (hc == NULL) {
        hc = new_context;
    }
    else {
        last = hc;
        while (last->next != NULL)
            last = last->next;
        last->next = new_context;
        new_context->prev = last;
    }
}

void remove_client (int client_fd)
{
    struct handler_context *elem;
    elem = hc;
    while (elem->client_fd != client_fd) {
        elem = elem->next;
    }

    if (elem == hc) {
        hc = elem->next;
        elem->prev = NULL;
    }
    else {
        elem->prev->next = elem->next;
        elem->next->prev = elem->prev;
    }
    free (elem);
}

void handle_client (int client_fd)
{
    struct handler_context *elem;
    size_t result_len;

    elem = hc;
    while (elem->client_fd != client_fd)
        elem = elem->next;
    if (elem == NULL) {
        perror ("elem not found");
        exit (1);
    }

    // todo: bug: when user send more than BUFFER_SIZE
    memset (elem->buffer, 0, BUFFER_SIZE);
    int bytes_read = read (client_fd, elem->buffer, BUFFER_SIZE - 1);
    if (bytes_read <= 0) {
        epoll_ctl (epfd, EPOLL_CTL_DEL, client_fd, NULL);
        close (client_fd);
        remove_client (client_fd);
        printf ("client disconnected\n");
    }

    // todo: bug: handle when command is longer than limit
    memcpy (elem->command + elem->command_cur, elem->buffer, bytes_read);
    elem->command_cur += bytes_read;
    int consume_count;
    while (1) {
        consume_count = consume_command (ht, elem->command, elem->result, &elem->tx_id);
        if (consume_count == 0)
            break;
        elem->command_cur = strlen (elem->command);

        if (elem->result[0] != '\0')
            strcpy (elem->result + strlen(elem->result), "\r\n");
        result_len = strlen (elem->result) + 1;

        if (write (client_fd, elem->result, result_len) < result_len) {
            perror ("failed to write");
            epoll_ctl (epfd, EPOLL_CTL_DEL, client_fd, NULL);
            close (client_fd);
            remove_client (client_fd);
            printf ("client disconnected\n");
        }
    }

    return;
}
