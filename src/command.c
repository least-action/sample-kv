#include "command.h"
#include "hash.h"
#include <string.h>
#include <stdbool.h>

// todo: use parser
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

bool is_command_del(char* command)
{
    return (strlen(command) > 4)
        && (command[0] == 'd')
        && (command[1] == 'e')
        && (command[2] == 'l')
        && (command[3] == ' ');
}

void run_command(char* command, char* result)
{
    char* buffer;

    if (is_command_empty(command)) {
        strcpy(result, "");
    }

    else if (is_command_get(command)) {
        buffer = hash_get_value(&ht, "hi");
        strcpy(result, buffer);
    }
    else if (is_command_set(command)) {
        strcpy(result, "OK");
    }
    else if (is_command_del(command)) {
        strcpy(result, "1");
    }
    else {
        strcpy(result, "Invalid command.");
    }
}

