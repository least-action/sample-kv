#include "kv_command.h"
#include "kv_hash.h"
#include "kv_redoundo.h"
#include "transaction.h"

#include <string.h>
#include <stdbool.h>
#include <stdio.h>


char* invalid_command = "invalid_command";
char* get_notfound = "(nil)";
char* set_success = "OK";
char* del_success = "1";
char* del_not_found = "0";
char* transaction_started = "started";
char* transaction_already_started = "transaction is ongoing";
char* transaction_committed = "commit";
char* transaction_aborted = "rollback";
char* transaction_not_started = "transaction is not ongoing";


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

void run_command(struct kv_ht *ht, char* command, char* result, int* tx_id)
{
    char *get_result;
    int del_result;
    bool is_single_command = false;

    if (is_command_empty(command)) {
        strcpy(result, "");
        return;
    }

    if (is_transaction_started (command)) {
        if (*tx_id > 0) {
            strcpy (result, transaction_already_started);
            return;
        }

        *tx_id = kv_tx_start_new_transaction ();
        kv_ru_add (*tx_id, KV_RU_BEGIN, NULL, NULL, NULL);
        strcpy (result, transaction_started);
        return;
    }

    if (is_command_get(command)) {
        // todo: does single-command-get require transaction?
        if (*tx_id == 0) {
            *tx_id = kv_tx_start_new_transaction ();
            kv_ru_add (*tx_id, KV_RU_BEGIN, NULL, NULL, NULL);
            is_single_command = true;
        }

        command[strlen(command)-2] = '\0';
        get_result = kv_ht_get (ht, command+4);
        if (get_result == NULL)
            strcpy (result, get_notfound);
        else
            strcpy (result, get_result);

        if (is_single_command) {
            // commit: todo: release locks
            kv_ru_add (*tx_id, KV_RU_COMMIT, NULL, NULL, NULL);
            kv_tx_end_transaction (*tx_id);
            *tx_id = 0;
        }
    }
    else if (is_command_set(command)) {
        if (*tx_id == 0) {
            *tx_id = kv_tx_start_new_transaction ();
            kv_ru_add (*tx_id, KV_RU_BEGIN, NULL, NULL, NULL);
            is_single_command = true;
        }

        int value_start = get_value_start(command);
        command[value_start-1] = '\0';
        char *key = command+4;
        char *value = command + value_start;
        value[strlen(value)-2] = '\0';
        get_result = kv_ht_get (ht, key);
        
        // todo: kv_ru_add and kv_ht_set pair should not be interleaving other lock sharing pair.
        //       If it occurs, current process and restored process are not same
        kv_ru_add (*tx_id, KV_RU_WRITE, key, value, get_result);

        kv_ht_set (ht, key, value);
        strcpy(result, set_success);

        if (is_single_command) {
            kv_ru_add (*tx_id, KV_RU_COMMIT, NULL, NULL, NULL);
            kv_tx_end_transaction (*tx_id);
            *tx_id = 0;
        }
    }
    else if (is_command_del(command)) {
        if (*tx_id == 0) {
            *tx_id = kv_tx_start_new_transaction ();
            kv_ru_add (*tx_id, KV_RU_BEGIN, NULL, NULL, NULL);
            is_single_command = true;
        }

        command[strlen(command)-2] = '\0';
        char *key = command+4;

        get_result = kv_ht_get (ht, key);

        kv_ru_add (*tx_id, KV_RU_DELETE, key, NULL, get_result);

        del_result = kv_ht_del (ht, key);
        if (del_result == 0)
            strcpy (result, del_not_found);
        else {
            strcpy (result, del_success);
        }

        if (is_single_command) {
            kv_ru_add (*tx_id, KV_RU_COMMIT, NULL, NULL, NULL);
            kv_tx_end_transaction (*tx_id);
            *tx_id = 0;
        }
    }
    else if (is_transaction_commited (command)) {
        if (*tx_id == 0) {
            strcpy (result, transaction_not_started);
        } else {
            kv_ru_add (*tx_id, KV_RU_COMMIT, NULL, NULL, NULL);
            kv_tx_end_transaction (*tx_id);
            *tx_id = 0;
            strcpy (result, transaction_committed);
        }
    }
    else if (is_transaction_aborted (command)) {
        if (*tx_id == 0) {
            strcpy (result, transaction_not_started);
        } else {
            kv_ru_add (*tx_id, KV_RU_ABORT, NULL, NULL, NULL);
            kv_ru_undo (*tx_id);
            kv_tx_end_transaction (*tx_id);
            *tx_id = 0;
            strcpy (result, transaction_aborted);
        }
    }
    else {
        strcpy(result, invalid_command);
    }
}

int consume_command(struct kv_ht *ht, char *command, char *result, int *tx_id)
{
    int end_of_command = find_end_of_command (command);
    if (end_of_command == -1)
        return 0;

    char temp[128];  // todo: refactor: remove magic number
    memcpy (temp, command, 128);
    memset (command, 0, 128);
    memcpy (command, temp + end_of_command + 1, strlen (temp) - end_of_command - 1);
    temp[end_of_command + 1] = '\0';

    run_command (ht, temp, result, tx_id);
    return 1;
}

