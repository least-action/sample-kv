#include <stdio.h>

#include "kv_server.h"

// todo: port from arg
#define PORT 1234

int main()
{
    // todo: feature: file io multiplexing
    // todo: feature: transaction
    // todo: bug: remove transaction context when client closed
    int server_termination_state;

    printf ("program started.\n");
    server_termination_state = kv_run_server (PORT);

    if (server_termination_state == 0)
        printf ("server sucessfully terminated.\n");
    else if (server_termination_state == 4)
        ;
    else
        printf("server terminated with %d\n", server_termination_state);

    return 0;
}

