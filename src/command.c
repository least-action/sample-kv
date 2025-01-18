#include "command.h"


size_t run_command(char* command, char* result)
{
    result[0] = 'O';
    result[1] = 'K';
    result[2] = '\r';
    result[3] = '\n';
    result[4] = '\0';

    return 5;
}

