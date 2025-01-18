#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <stddef.h>

#include "command.h"

#define PORT 1234
#define BUFFER_SIZE 64
#define RESULT_SIZE 64

#define MAX_EVENTS 64

int main()
{
    printf ("program started.\n");

    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof (client_addr);
    char buffer[BUFFER_SIZE];
    char result[RESULT_SIZE];
    size_t result_len;

    struct epoll_event set_event;
    int epfd;
    struct epoll_event *events;
    int e_count;

    server_fd = socket (AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror ("failed to create server socket");
        exit (EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

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
  

    printf("server is running on port %d\n", PORT);

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

                if (write (client_fd, "kv> ", 4) < 4) {
                    perror ("failed to write");
                    epoll_ctl (epfd, EPOLL_CTL_DEL, client_fd, NULL);
                    close (client_fd);
                    printf ("client disconnected\n");
                }
            }
           
            else {
                client_fd = events[i].data.fd;
                memset (buffer, 0, BUFFER_SIZE);
                int bytes_read = read (client_fd, buffer, BUFFER_SIZE - 1);
                if (bytes_read <= 0) {
                    epoll_ctl (epfd, EPOLL_CTL_DEL, client_fd, NULL);
                    close (client_fd);
                    printf ("client disconnected\n");
                    continue;
                }

                result_len = run_command(buffer, result);

                if (write (client_fd, result, result_len) < result_len) {
                    perror ("failed to write");
                    epoll_ctl (epfd, EPOLL_CTL_DEL, client_fd, NULL);
                    close (client_fd);
                    printf ("client disconnected\n");
                }

                if (write (client_fd, "kv> ", 4) < 4) {
                    perror ("failed to write");
                    epoll_ctl (epfd, EPOLL_CTL_DEL, client_fd, NULL);
                    close (client_fd);
                    printf ("client disconnected\n");
                }
            }
        }
    }

    free (events);
    close (epfd);
    close (server_fd);

    printf ("server sucessfully terminated.\n");

    return 0;
}

