#include "kv_command.h"
#include "kv_hash.h"
#include "kv_redoundo.h"
#include "transaction.h"
#include "kv_server.h"
#include "kv_client_handler.h"
#include "snapshot_thread.h"
#include "shutdown.h"
#include "lock_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stddef.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>

struct kv_ht *ht;

void cleanup ()
{
    // todo: what todo?
}

void handle_sigint (int sig)
{
    is_running = 0;
}


int kv_run_server (uint16_t port)
{
    if (signal (SIGINT, handle_sigint) == SIG_ERR) {
        perror("failed to register SIGINT handler");
        return 1;
    }
    if (atexit(cleanup) != 0) {
        fprintf(stderr, "failted to register cleanup on atexit\n");
        return 1;
    }

    int server_fd, client_fd;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof (client_addr);

    pthread_t snapshot_thread;
    char snapshot_thread_return;
    pthread_cond_t snapshot_terminate_cond;
    pthread_mutex_t snapshot_terminate_lock;
    struct kv_snapshot_arg *snapshot_arg;

    pthread_t thread;
    int ret;
    struct kv_handle_client_data *data;
    struct kv_lm *lm;

    ht = kv_ht_create (2, str_hash_func, str_cmp_func);

    kv_ru_init ();
    kv_tx_init ();
    kv_ru_redo (ht);
    kv_tx_init2 ();

    lm = kv_lm_create ();

    pthread_mutex_init (&snapshot_terminate_lock, NULL);
    pthread_cond_init (&snapshot_terminate_cond, NULL);
    snapshot_arg = (struct kv_snapshot_arg *) malloc (sizeof (struct kv_snapshot_arg));
    snapshot_arg->terminate_cond = &snapshot_terminate_cond;
    snapshot_arg->terminate_lock = &snapshot_terminate_lock;
    ret = pthread_create (&snapshot_thread, NULL, (void*) kv_snapshot_thread, snapshot_arg);
    if (ret != 0) {
        errno = ret;
        perror ("pthread(snapshot) create error");
    }

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

    while (is_running) {
        client_fd = accept (server_fd, (struct sockaddr*)&client_addr, &addr_len);  // todo: handle when is_running changed to false
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

        data = (struct kv_handle_client_data *) malloc (sizeof (struct kv_handle_client_data));
        data->client_fd = client_fd;
        data->lm = lm;
        ret = pthread_create (&thread, NULL, (void*) kv_handle_client, data);  // todo: join?
        if (ret != 0) {
            errno = ret;
            perror ("pthread create error");
            close (client_fd);
            continue;
        }
    }

    pthread_mutex_lock (&snapshot_terminate_lock);
    pthread_cond_signal (&snapshot_terminate_cond);
    pthread_mutex_unlock (&snapshot_terminate_lock);
    pthread_join (snapshot_thread, (void *) &snapshot_thread_return);
    free (snapshot_arg);
    // todo: signal and join client handler threads
    kv_lm_destruct (lm);

    close (server_fd);
    kv_ru_destroy ();

    return 0;
}

