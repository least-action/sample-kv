#include <stdio.h>
#include <string.h>

#include "kv_server.h"
#include "utils.h"

#define DEFAULT_PORT 1234

enum arg_state {
    KV_ARG_READY,
    KV_ARG_PORT_SETTING,
};

int main(int argc, char* argv[])
{
    // todo: feature: transaction
    // todo: use parser
    // todo: add delete all command
    // todo: terminate main thread when child thread exited wtih error
    // todo: remove exit(1)

    int server_termination_state;
    enum arg_state state = KV_ARG_READY;
    char* arg;
    int port = 0;

    for (int i = 1; i < argc; ++i) {
        arg = argv[i];

        if (state == KV_ARG_READY) {
            if (strcmp ("-p", arg) == 0) {
                state = KV_ARG_PORT_SETTING;
            }
        }
        else if (state == KV_ARG_PORT_SETTING) {
            port = digit_to_int (arg, strlen (arg));
        }
    }

    if (port == 0)
        port = DEFAULT_PORT;

    printf ("program started.\n");
    server_termination_state = kv_run_server (port);

    if (server_termination_state == 0)
        printf ("server sucessfully terminated.\n");
    else if (server_termination_state == 4)
        ;
    else
        printf("server terminated with %d\n", server_termination_state);

    return 0;
}

