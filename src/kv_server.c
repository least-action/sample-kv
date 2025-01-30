#include "kv_command.h"
#include "kv_hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <stddef.h>

#define BUFFER_SIZE 64
#define COMMAND_SIZE 64
#define RESULT_SIZE 64
#define MAX_EVENTS 64

static int count;
static struct kv_ht *ht;
static int epfd;

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
    char result[RESULT_SIZE];
    struct handler_context *prev;  // todo: perf: optimize O(n)
    struct handler_context *next;
};

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
        consume_count = consume_command (ht, elem->command, elem->result);
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

int kv_run_server (uint16_t port)
{
    count = 0;
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof (client_addr);

    struct epoll_event set_event;
    struct epoll_event *events;
    int e_count;

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

    epfd = epoll_create1(EPOLL_CLOEXEC);
    if (epfd < 0) {
        perror ("epoll_create1 error");
        close (server_fd);
        exit (EXIT_FAILURE);
    }

    set_event.data.fd = server_fd;
    set_event.events = EPOLLIN;
    if (epoll_ctl (epfd, EPOLL_CTL_ADD, server_fd, &set_event)) {
        perror ("epoll_ctl error");
        close (epfd);
        close (server_fd);
        exit (EXIT_FAILURE);
    }

    events = malloc (sizeof(struct epoll_event) * MAX_EVENTS);
    if (!events) {
        perror ("epoll malloc");
        free (events);
        close (epfd);
        close (server_fd);
        exit (EXIT_FAILURE);
    }

    printf ("server is running on port %d\n", port);

    ht = kv_ht_create (2);

    while (1) {
        e_count = epoll_wait (epfd, events, MAX_EVENTS, -1);
        if (e_count < 0) {
            perror ("epoll_wait");
            break;
        }

        for (int i = 0; i < e_count; ++i) {
            if (events[i].data.fd == server_fd) {
                client_fd = accept (server_fd, (struct sockaddr*)&client_addr, &addr_len);
                 if (client_fd < 0) {
                    perror("failed to accept");
                    continue;
                }

                set_event.data.fd = client_fd;
                set_event.events = EPOLLIN;
                epoll_ctl (epfd, EPOLL_CTL_ADD, client_fd, &set_event);

                printf ("client connected from: %s:%d\n",
                    inet_ntoa (client_addr.sin_addr),
                    ntohs (client_addr.sin_port)
                );

                if (write (client_fd, "connected\r\n", 11) < 11) {
                    perror ("failed to write");
                    epoll_ctl (epfd, EPOLL_CTL_DEL, client_fd, NULL);
                    close (client_fd);
                    printf ("client disconnected\n");
                }

                register_client (client_fd);
            }
           else {
                client_fd = events[i].data.fd;
                handle_client (client_fd);
            }
        }
    }

    free (events);
    close (epfd);
    close (server_fd);

    return 0;
}

