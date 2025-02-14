#include "kv_command.h"
#include "kv_hash.h"
#include "kv_redoundo.h"

#include <string.h>
#include <stdbool.h>
#include <stdio.h>


char* invalid_command = "invalid_command";
char* get_notfound = "(nil)";
char* set_success = "OK";
char* del_success = "1";
char* del_not_found = "0";
char* transaction_started = "started";
char* transaction_committed = "commit";
char* transaction_aborted = "rollback";

struct transaction_data {
    int transaction_id;
    int start_id;
    struct transaction_data *next;
};

struct transaction_data *td_list;

// todo: use parser
bool is_command_empty(char* command)
{
    return (strlen(command) == 2) && (command[0] == '\r') && (command[1] == '\n');
}

// todo: add delete all command
bool is_command_get(char* command)
{
    return (strlen(command) > 4)
        && (command[0] == 'g')
        && (command[1] == 'e')
        && (command[2] == 't')
        && (command[3] == ' ');
}

int space_count (char *command)
{
    int count = 0;
    int idx = 0;
    char c;

    while ((c = command[idx++])) {
        if (c == ' ')
            ++count;
    }
    return count;
}

bool is_command_set (char* command)
{
    return (strlen(command) > 4)
        && (command[0] == 's')
        && (command[1] == 'e')
        && (command[2] == 't')
        && (command[3] == ' ')
        && (space_count (command) == 2);
}

bool is_command_del (char* command)
{
    return (strlen(command) > 4)
        && (command[0] == 'd')
        && (command[1] == 'e')
        && (command[2] == 'l')
        && (command[3] == ' ');
}

bool is_transaction_started (char* command)
{
    return (strlen(command) ==  7)
        && (command[0] == 'b')
        && (command[1] == 'e')
        && (command[2] == 'g')
        && (command[3] == 'i')
        && (command[4] == 'n');
}

bool is_transaction_commited (char* command)
{
    return (strlen(command) == 8)
        && (command[0] == 'c')
        && (command[1] == 'o')
        && (command[2] == 'm')
        && (command[3] == 'm')
        && (command[4] == 'i')
        && (command[5] == 't');
}

bool is_transaction_aborted (char* command)
{
    return (strlen(command) == 7)
        && (command[0] == 'a')
        && (command[1] == 'b')
        && (command[2] == 'o')
        && (command[3] == 'r')
        && (command[4] == 't');
}

int get_value_start (char* command)
{
    // todo: check value is valid string
    int cur = 4;
    while (command[cur++] != ' ');
    return cur;
}

int find_end_of_command (char *buffer)
{
    int size = strlen (buffer);
    if (size < 2)
        return -1;

    for (int i = 0; i < size - 1; ++i) {
        if (buffer[i] == '\r' && buffer[i+1] == '\n')
            return i+1;
    }

    return -1;
}

void run_command(struct kv_ht *ht, char* command, char* result)
{
    char *get_result;
    int del_result;

    if (is_command_empty(command)) {
        strcpy(result, "");
    }

    // get transaction data

    else if (is_command_get(command)) {
        command[strlen(command)-2] = '\0';
        get_result = kv_ht_get (ht, command+4);
        if (get_result == NULL)
            strcpy (result, get_notfound);
        else
            strcpy (result, get_result);
    }
    else if (is_command_set(command)) {
        int value_start = get_value_start(command);
        command[value_start-1] = '\0';
        char *key = command+4;
        char *value = command + value_start;
        value[strlen(value)-2] = '\0';
        
        kv_redo_add (REDO_SET, key, value);

        get_result = kv_ht_get (ht, command+4);
        if (get_result == NULL) {
            kv_undo_add (UNDO_DEL, key, NULL);
        }
        else {
            kv_undo_add (UNDO_SET, key, get_result);
        }

        kv_ht_set (ht, key, value);
        strcpy(result, set_success);
    }
    else if (is_command_del(command)) {
        command[strlen(command)-2] = '\0';
        char *key = command+4;

        kv_redo_add (REDO_DEL, key, NULL);

        get_result = kv_ht_get (ht, key);

        del_result = kv_ht_del (ht, key);
        if (del_result == 0)
            strcpy (result, del_not_found);
        else {
            kv_undo_add (UNDO_SET, key, get_result);
            strcpy (result, del_success);
        }
    }
    else if (is_transaction_started (command)) {
        strcpy (result, transaction_started);
    }
    else if (is_transaction_commited (command)) {
        strcpy (result, transaction_committed);
    }
    else if (is_transaction_aborted (command)) {
        strcpy (result, transaction_aborted);
    }
    else {
        strcpy(result, invalid_command);
    }
}

int consume_command(struct kv_ht *ht, char *command, char *result, int *transaction_id)
{
    int end_of_command = find_end_of_command (command);
    if (end_of_command == -1)
        return 0;

    char temp[128];  // todo: refactor: remove magic number
    memcpy (temp, command, 128);
    memset (command, 0, 128);
    memcpy (command, temp + end_of_command + 1, strlen (temp) - end_of_command - 1);
    temp[end_of_command + 1] = '\0';

    run_command (ht, temp, result);
    return 1;
}

