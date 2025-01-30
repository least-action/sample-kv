#include <stdio.h>

#include "kv_server.h"

// todo: port from arg
#define PORT 1234

int main()
{
    // todo: feature: transaction
    // todo: feature: persistence
    int server_termination_state;

    printf ("program started.\n");
    server_termination_state = kv_run_server (PORT);

    if (server_termination_state == 0)
        printf ("server sucessfully terminated.\n");
    else
        printf("server terminated with %d\n", server_termination_state);

    return 0;
}

