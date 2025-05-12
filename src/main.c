// #include <stdio.h>
// #include <string.h>

// #include "kv_server.h"
// #include "utils.h"

// #define DEFAULT_PORT 1234

// enum arg_state {
//     KV_ARG_READY,
//     KV_ARG_PORT_SETTING,
// };

// int main(int argc, char* argv[])
// {
//     // todo: feature: transaction
//     // todo: use parser
//     // todo: add delete all command
//     // todo: terminate main thread when child thread exited wtih error
//     // todo: remove exit(1)

//     int server_termination_state;
//     enum arg_state state = KV_ARG_READY;
//     char* arg;
//     int port = 0;

//     for (int i = 1; i < argc; ++i) {
//         arg = argv[i];

//         if (state == KV_ARG_READY) {
//             if (strcmp ("-p", arg) == 0) {
//                 state = KV_ARG_PORT_SETTING;
//             }
//         }
//         else if (state == KV_ARG_PORT_SETTING) {
//             port = digit_to_int (arg, strlen (arg));
//         }
//     }

//     if (port == 0)
//         port = DEFAULT_PORT;

//     printf ("program started.\n");
//     server_termination_state = kv_run_server (port);

//     if (server_termination_state == 0)
//         printf ("server sucessfully terminated.\n");
//     else if (server_termination_state == 4)
//         ;
//     else
//         printf("server terminated with %d\n", server_termination_state);

//     return 0;
// }



#include "lock_rwlock.h"

#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

struct kv_rwl *lock;


int a_l1_finished = 0;
pthread_mutex_t end_a = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t end_a_cond = PTHREAD_COND_INITIALIZER;
int b_l1_finished = 0;
pthread_mutex_t end_b = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t end_b_cond = PTHREAD_COND_INITIALIZER;


void* aa (void *data)
{
    printf("-------- test 1 started --------\n");
    usleep (1000000);
    printf("aa: try to get rlock\n");
    kv_rwl_rlock (lock);
    printf("aa: success\n");
    usleep (1000000);
    printf("aa: try to get rlock\n");
    kv_rwl_rlock (lock);
    printf("aa: success\n");
    usleep (1000000);
    printf("aa: try to get wlock\n");
    kv_rwl_wlock (lock);
    printf("aa: success\n");
    usleep (1000000);
    printf("aa: try to get unlock\n");
    kv_rwl_unlock (lock);
    printf("aa: success\n");
    printf("aa: try to get unlock\n");
    kv_rwl_unlock (lock);
    printf("aa: success\n");
    printf("aa: try to get unlock\n");
    kv_rwl_unlock (lock);
    printf("aa: success\n");

    pthread_mutex_lock (&end_a);
    {
        a_l1_finished = 1;
        pthread_cond_signal (&end_a_cond);
    }
    pthread_mutex_unlock (&end_a);
    pthread_mutex_lock (&end_b);
    {
        while (!b_l1_finished)
            pthread_cond_wait (&end_b_cond, &end_b);
    }
    pthread_mutex_unlock (&end_b);
    usleep (1000000);
    pthread_mutex_lock (&end_a);
    {
        a_l1_finished = 0;
    }
    pthread_mutex_unlock (&end_a);

    printf("-------- test 2 started --------\n");
    usleep (1000000);
    printf("aa: try to get wlock\n");
    kv_rwl_wlock (lock);
    printf("aa: success\n");
    usleep (1000000);
    printf("aa: try to get rlock\n");
    kv_rwl_rlock (lock);
    printf("aa: success\n");
    usleep (1000000);
    printf("aa: try to get unlock\n");
    kv_rwl_unlock (lock);
    printf("aa: success\n");
    printf("aa: try to get unlock\n");
    kv_rwl_unlock (lock);
    printf("aa: success\n");

    pthread_mutex_lock (&end_a);
    {
        a_l1_finished = 1;
        pthread_cond_signal (&end_a_cond);
    }
    pthread_mutex_unlock (&end_a);
    pthread_mutex_lock (&end_b);
    {
        while (!b_l1_finished)
            pthread_cond_wait (&end_b_cond, &end_b);
    }
    pthread_mutex_unlock (&end_b);
    usleep (1000000);
    pthread_mutex_lock (&end_a);
    {
        a_l1_finished = 0;
    }
    pthread_mutex_unlock (&end_a);
    
    printf("-------- test 3 started --------\n");

    usleep (1000000);
    printf("aa: try to get rlock\n");
    kv_rwl_rlock (lock);
    printf("aa: success\n");
    usleep (1000000);
    printf("aa: try to get wlock\n");
    kv_rwl_wlock (lock);
    printf("aa: success\n");
    usleep (1000000);
    printf("aa: try to get unlock\n");
    kv_rwl_unlock (lock);
    printf("aa: success\n");
    printf("aa: try to get unlock\n");
    kv_rwl_unlock (lock);
    printf("aa: success\n");

    
    printf("aa: finished\n");
    
    return NULL;
}

void* bb (void *data)
{
    char btab[] = "                                ";
    
    printf("%s-------- test 1 started --------\n", btab);
    usleep (1500000);
    printf("%sbb: try to get wlock\n", btab);
    kv_rwl_wlock (lock);
    printf("%sbb: success\n", btab);
    usleep (1000000);
    printf("%sbb: try to get unlock\n", btab);
    kv_rwl_unlock (lock);
    printf("%sbb: success\n", btab);

    pthread_mutex_lock (&end_b);
    {
        b_l1_finished = 1;
        pthread_cond_signal (&end_b_cond);
    }
    pthread_mutex_unlock (&end_b);
    pthread_mutex_lock (&end_a);
    {
        while (!a_l1_finished)
            pthread_cond_wait (&end_a_cond, &end_a);
    }
    pthread_mutex_unlock (&end_a);
    usleep (1000000);
    pthread_mutex_lock (&end_b);
    {
        b_l1_finished = 0;
    }
    pthread_mutex_unlock (&end_b);

    printf("%s-------- test 2 started --------\n", btab);
    usleep (1500000);
    printf("%sbb: try to get rlock\n", btab);
    kv_rwl_rlock (lock);
    printf("%sbb: success\n", btab);
    usleep (1000000);
    printf("%sbb: try to get unlock\n", btab);
    kv_rwl_unlock (lock);
    printf("%sbb: success\n", btab);

    pthread_mutex_lock (&end_b);
    {
        b_l1_finished = 1;
        pthread_cond_signal (&end_b_cond);
    }
    pthread_mutex_unlock (&end_b);
    pthread_mutex_lock (&end_a);
    {
        while (!a_l1_finished)
            pthread_cond_wait (&end_a_cond, &end_a);
    }
    pthread_mutex_unlock (&end_a);
    usleep (1000000);
    pthread_mutex_lock (&end_b);
    {
        b_l1_finished = 0;
    }
    pthread_mutex_unlock (&end_b);
    

    printf("%s-------- test 3 started --------\n", btab);

    usleep (1000000);
    printf("%sbb: try to get rlock\n", btab);
    kv_rwl_rlock (lock);
    printf("%sbb: success\n", btab);
    usleep (1500000);
    printf("%sbb: try to get wlock\n", btab);
    kv_rwl_wlock (lock);
    printf("%sbb: success\n", btab);
    usleep (1000000);
    printf("%sbb: try to get unlock\n", btab);
    kv_rwl_unlock (lock);
    printf("%sbb: success\n", btab);
    printf("%sbb: try to get unlock\n", btab);
    kv_rwl_unlock (lock);
    printf("%sbb: success\n", btab);


    printf("%sbb: finished\n", btab);
    
    return NULL;
}


int main()
{
    int ret;
    pthread_t thread1;
    pthread_t thread2;
    void* return_value;

    lock = kv_rwl_create ();
    
    // pthread_mutex_init (&cond_mutex, NULL);
    // pthread_cond_init (&cond1, NULL);


    ret = pthread_create (&thread1, NULL, (void*) aa, NULL);
    if (ret != 0) {
        errno = ret;
        perror ("pthread create error");
    }

    ret = pthread_create (&thread2, NULL, (void*) bb, NULL);
    if (ret != 0) {
        errno = ret;
        perror ("pthread create error");
    }

    pthread_join (thread1, &return_value);
    pthread_join (thread2, &return_value);
}
