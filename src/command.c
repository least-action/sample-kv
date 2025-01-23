#include "command.h"
#include "hash.h"
#include <string.h>
#include <stdbool.h>
#include <stdio.h>


char* invalid_command = "invalid_command";
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

int get_value_start(char* command)
{
    // todo: check value is valid string
    int cur = 4;
    while (command[cur++] != ' ');
    return cur;
}

void run_command(char* command, char* result)
{
    char* buffer;

    if (is_command_empty(command)) {
        strcpy(result, "");
    }

    else if (is_command_get(command)) {
        command[strlen(command)-2] = '\0';
        buffer = hash_get_value(&ht, command+4);
        strcpy(result, buffer);
    }
    else if (is_command_set(command)) {
        int value_start = get_value_start(command);
        command[value_start-1] = '\0';
        char *key = command+4;
        char *value = command + value_start;
        value[strlen(value)-2] = '\0';
        
        buffer = hash_set_value(&ht, key, value);
        strcpy(result, buffer);
    }
    else if (is_command_del(command)) {
        command[strlen(command)-2] = '\0';
        buffer = hash_del_value(&ht, command+4);
        strcpy(result, buffer);
    }
    else {
        strcpy(result, invalid_command);
    }
}

