#include "command.h"
#include <string.h>
#include <stdbool.h>

struct result_get {
    size_t result_len;
    char* result;
};

struct result_get get_value(char* key)
{
}

bool is_command_empty(char* command)
{
    return (strlen(command) == 2) && (command[0] == '\r') && (command[1] == '\n');
}

bool is_command_get(char* command)
{
    return (strlen(command) > 4)
        && (command[0] == 'g')
        && (command[1] == 'e')
        && (command[2] == 't')
        && (command[3] == ' ');
}

bool is_command_set(char* command)
{
    return (strlen(command) > 4)
        && (command[0] == 's')
        && (command[1] == 'e')
        && (command[2] == 't')
        && (command[3] == ' ');
}

size_t run_command(char* command, char* result)
{
    if (is_command_empty(command)) {
        strcpy(result, "");
        return 1;
    }

    if (is_command_get(command)) {
        strcpy(result, "result\r\n");
        return 9;
    }

    if (is_command_set(command)) {
        strcpy(result, "OK\r\n");
        return 5;
    }

    strcpy(result, "Invalid command.\r\n");
    return 19;
}

