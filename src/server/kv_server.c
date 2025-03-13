#include "kv_command.h"
#include "kv_hash.h"
#include "kv_redoundo.h"
#include "server/kv_server.h"
#include "server/kv_client_handler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <stddef.h>

int epfd;
int server_fd;
struct kv_ht *ht;

void handle_event (struct epoll_event ee)
{
    int client_fd;
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof (client_addr);
    struct epoll_event set_event;

    if (ee.data.fd == server_fd) {
        client_fd = accept (server_fd, (struct sockaddr*)&client_addr, &addr_len);
        if (client_fd < 0) {
            perror("failed to accept");
            return;
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
        handle_client (ee.data.fd);
    }
    return;
}

int kv_run_server (uint16_t port)
{
    struct sockaddr_in server_addr;
    
    struct epoll_event set_event;
    struct epoll_event *events;
    int e_count;

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

    while (1) {
        e_count = epoll_wait (epfd, events, MAX_EVENTS, -1);
        if (e_count < 0) {
            perror ("epoll_wait");
            break;
        }

        for (int i = 0; i < e_count; ++i)
            handle_event(events[i]);
    }

    free (events);
    close (epfd);
    close (server_fd);
    kv_redo_terminate ();

    return 0;
}

