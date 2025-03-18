#include "kv_command.h"
#include "kv_hash.h"
#include "kv_redoundo.h"
#include "server/kv_server.h"
#include "el_event_loop.h"
#include "kv_shared.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stddef.h>

#define BUFFER_SIZE 128
#define COMMAND_SIZE 128
#define RESULT_SIZE 64


struct kv_ht *ht;
// struct handler_context *hc;

enum client_context_state {
    KV_CCS_STARTED,
};

struct client_context {
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
};

void handle_client_fd (struct kv_event_loop* el, int client_fd, void *data)
{
    struct client_context* client_context = (struct client_context*) data;
    size_t result_len;

    // todo: bug: when user send more than BUFFER_SIZE
    memset (client_context->buffer, 0, BUFFER_SIZE);
    int bytes_read = read (client_fd, client_context->buffer, BUFFER_SIZE - 1);
    if (bytes_read <= 0) {
        kv_el_delete_file_event (el, client_fd, EL_READABLE);
        close (client_fd);
        remove_client (client_fd);
        printf ("client disconnected\n");
    }

    // todo: bug: handle when command is longer than limit
    memcpy (client_context->command + client_context->command_cur, client_context->buffer, bytes_read);
    client_context->command_cur += bytes_read;
    int consume_count;
    while (1) {
        consume_count = consume_command (ht, client_context->command, client_context->result, &client_context->tx_id);
        if (consume_count == 0)
            break;
            client_context->command_cur = strlen (client_context->command);

        if (client_context->result[0] != '\0')
            strcpy (client_context->result + strlen(client_context->result), "\r\n");
        result_len = strlen (client_context->result) + 1;

        if (write (client_fd, client_context->result, result_len) < result_len) {
            perror ("failed to write");
            kv_el_delete_file_event (el, client_fd, EL_READABLE);
            close (client_fd);
            remove_client (client_fd);
            printf ("client disconnected\n");
        }
    }

    return;
}


struct client_context* create_new_clilent_data ()
{
    struct client_context *client_context;
    client_context = (struct client_context*) malloc (sizeof (struct client_context));
    memset (client_context->buffer, 0, BUFFER_SIZE);
    memset (client_context->command, 0, COMMAND_SIZE);
    client_context->command_cur = 0;
    client_context->tx_id = -1;
    memset (client_context->result, 0, RESULT_SIZE);
    
    return client_context;
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

void handle_server_fd (struct kv_event_loop* el, int server_fd, void* data)
{
    UNUSED (data);
    
    int client_fd;
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof (client_addr);

    client_fd = accept (server_fd, (struct sockaddr*)&client_addr, &addr_len);
    if (client_fd < 0) {
        perror("failed to accept");
        return;
    }

    printf ("client connected from: %s:%d\n",
        inet_ntoa (client_addr.sin_addr),
        ntohs (client_addr.sin_port)
    );

    if (write (client_fd, "connected\r\n", 11) < 11) {
        perror ("failed to write");
        kv_el_delete_file_event (el, client_fd, EL_READABLE);
        close (client_fd);
        printf ("client disconnected\n");
    }

    struct client_context* data = create_new_clilent_data ();

    kv_el_create_file_event (el, client_fd, handle_client_fd, (void*) data, EL_READABLE);
}

int kv_run_server (uint16_t port)
{
    int server_fd;
    struct sockaddr_in server_addr;

    ht = kv_ht_create (2);
    kv_redo_init ();
    kv_redo_redo (ht);

    server_fd = socket (AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror ("failed to create server socket");
        exit (EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind (server_fd, (struct sockaddr*)&server_addr, sizeof (server_addr)) < 0) {
        perror ("failed to bind");
        close (server_fd);
        exit (EXIT_FAILURE);
    }

    if (listen (server_fd, 1) < 0) {
        perror ("failed to listen");
        close (server_fd);
        exit (EXIT_FAILURE);
    }

    printf ("server is running on port %d\n", port);

    struct kv_event_loop* el = kv_el_create_event_loop ();
    kv_el_create_file_event (el, server_fd, handle_server_fd, NULL, EL_READABLE);
    kv_el_process_event_loop (el);

    kv_redo_terminate ();  // todo: run on panic

    return 0;
}

