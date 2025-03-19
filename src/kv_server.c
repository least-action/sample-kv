#include "kv_command.h"
#include "kv_hash.h"
#include "kv_redoundo.h"
#include "kv_server.h"
#include "kv_client_handler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stddef.h>

struct kv_ht *ht;


int kv_run_server (uint16_t port)
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof (client_addr);
    pid_t pid;
    
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

    while (1) {
        client_fd = accept (server_fd, (struct sockaddr*)&client_addr, &addr_len);
        if (client_fd < 0) {
            perror("failed to accept");
            break;
        }

        printf ("client connected from: %s:%d\n",
            inet_ntoa (client_addr.sin_addr),
            ntohs (client_addr.sin_port)
        );

        if (write (client_fd, "connected\r\n", 11) < 11) {
            perror ("failed to write");
            close (client_fd);
            printf ("client disconnected\n");
        }

        pid = fork();
        if (pid == -1) {
            perror ("failed to fork");
            close (server_fd);
            close (client_fd);
            exit (EXIT_FAILURE);
        }

        if (pid == 0) {  // child
            close (server_fd);
            kv_handle_client (client_fd);
            return 4;
            
        } else {  // parent
            close (client_fd);
            continue;
        }
    }

    close (server_fd);
    kv_redo_terminate ();

    return 0;
}

