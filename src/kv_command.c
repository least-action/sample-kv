#include "kv_command.h"
#include "kv_hash.h"
#include "kv_redoundo.h"
#include "transaction.h"
#include "lock_manager.h"
#include "utils.h"

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

struct key_data {
    char *key;
    size_t key_len;
};

struct val_data {
    char *value;
    size_t val_len;
};

bool is_command_empty(const char* command)
{
    return (strlen(command) == 2) && (command[0] == '\r') && (command[1] == '\n');
}

bool is_command_get(const char* command)
{
    return (strlen(command) > 4)
        && (command[0] == 'g')
        && (command[1] == 'e')
        && (command[2] == 't')
        && (command[3] == ' ');
}

int space_count (const char *command)
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

bool is_command_set (const char* command)
{
    return (strlen(command) > 4)
        && (command[0] == 's')
        && (command[1] == 'e')
        && (command[2] == 't')
        && (command[3] == ' ')
        && (space_count (command) == 2);
}

bool is_command_del (const char* command)
{
    return (strlen(command) > 4)
        && (command[0] == 'd')
        && (command[1] == 'e')
        && (command[2] == 'l')
        && (command[3] == ' ');
}

bool is_transaction_started (const char* command)
{
    return (strlen(command) ==  7)
        && (command[0] == 'b')
        && (command[1] == 'e')
        && (command[2] == 'g')
        && (command[3] == 'i')
        && (command[4] == 'n');
}

bool is_transaction_commited (const char* command)
{
    return (strlen(command) == 8)
        && (command[0] == 'c')
        && (command[1] == 'o')
        && (command[2] == 'm')
        && (command[3] == 'm')
        && (command[4] == 'i')
        && (command[5] == 't');
}

bool is_transaction_aborted (const char* command)
{
    return (strlen(command) == 7)
        && (command[0] == 'a')
        && (command[1] == 'b')
        && (command[2] == 'o')
        && (command[3] == 'r')
        && (command[4] == 't');
}

size_t get_key_len_of_set (const char *command)
{
    size_t cur = 4;
    while (command[++cur] != ' ');  // todo: bug: check key is not empty
    return cur - 4;
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

void run_command(struct kv_ht *ht, struct kv_lm *lm, const char* command, const size_t command_len, char* result, int* tx_id)
{
    char *key = NULL;
    char *value = NULL;
    size_t key_len;
    size_t value_len;
    struct key_data *k_data = NULL;
    struct key_data kd = { NULL, 0 };
    struct val_data *v_data = NULL;
    struct val_data *old_v_data = NULL;
    struct kv_ht_kv old_kv = { NULL, NULL };

    bool is_single_command = false;
    struct kv_rwl *rwl = NULL;

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
        kv_ru_add (*tx_id, KV_RU_BEGIN, NULL, 0, NULL, 0, NULL, 0);
        strcpy (result, transaction_started);
        return;
    }

    if (is_command_get(command)) {
        // todo: does single-command-get require transaction?
        if (*tx_id == 0) {
            *tx_id = kv_tx_start_new_transaction ();
            kv_ru_add (*tx_id, KV_RU_BEGIN, NULL, 0, NULL, 0, NULL, 0);
            is_single_command = true;
        }

        key_len = command_len - 4;
        key = (char *) malloc (key_len);
        memcpy (key, command + 4, key_len);
        kd.key = key;
        kd.key_len = key_len;

        rwl = kv_lm_get_rwlock (lm, key, key_len);
        kv_rwl_rlock (rwl);
        {
            v_data = kv_ht_get (ht, &kd);
        }
        kv_rwl_un_rlock (rwl);

        free (key);

        if (v_data == NULL)
            strcpy (result, get_notfound);
        else {
            memcpy (result, v_data->value, v_data->val_len);
            result[v_data->val_len] = '\0';
        }

        if (is_single_command) {
            // commit: todo: release locks
            kv_ru_add (*tx_id, KV_RU_COMMIT, NULL, 0, NULL, 0, NULL, 0);
            kv_tx_end_transaction (*tx_id);
            *tx_id = 0;
        }
    }
    else if (is_command_set(command)) {
        if (*tx_id == 0) {
            *tx_id = kv_tx_start_new_transaction ();
            kv_ru_add (*tx_id, KV_RU_BEGIN, NULL, 0, NULL, 0, NULL, 0);
            is_single_command = true;
        }

        key_len = get_key_len_of_set (command);
        key = (char *) malloc (sizeof (char) * key_len);
        memcpy (key, command+4, key_len);

        value_len = command_len - key_len - 5;
        value = (char *) malloc (sizeof (char) * value_len);
        memcpy (value, command + key_len + 5, value_len);

        k_data = (struct key_data *) malloc (sizeof (struct key_data));
        k_data->key = key;
        k_data->key_len = key_len;
        v_data = (struct val_data *) malloc (sizeof (struct val_data));
        v_data->value = value;
        v_data->val_len = value_len;

        rwl = kv_lm_get_rwlock (lm, key, key_len);
        kv_rwl_wlock (rwl);
        {
            old_v_data = kv_ht_get (ht, k_data);
            // todo: bug: key data malloc free when updated
            if (old_v_data == NULL)
                kv_ru_add (*tx_id, KV_RU_WRITE, k_data->key, k_data->key_len, v_data->value, v_data->val_len, NULL, 0);
            else
                kv_ru_add (*tx_id, KV_RU_WRITE, k_data->key, k_data->key_len, v_data->value, v_data->val_len, old_v_data->value, old_v_data->val_len);
            old_v_data = kv_ht_set (ht, k_data, v_data);
        }
        kv_rwl_un_wlock (rwl);

        if (old_v_data != NULL) {
            free (old_v_data->value);
            free (old_v_data);
        }
        strcpy(result, set_success);

        if (is_single_command) {
            kv_ru_add (*tx_id, KV_RU_COMMIT, NULL, 0, NULL, 0, NULL, 0);
            kv_tx_end_transaction (*tx_id);
            *tx_id = 0;
        }
    }
    else if (is_command_del(command)) {
        if (*tx_id == 0) {
            *tx_id = kv_tx_start_new_transaction ();
            kv_ru_add (*tx_id, KV_RU_BEGIN, NULL, 0, NULL, 0, NULL, 0);
            is_single_command = true;
        }

        key_len = command_len - 4;  // todo: bug: check +-1
        key = (char *) malloc (key_len);
        memcpy (key, command + 4, key_len);

        kd.key = key;
        kd.key_len = key_len;

        rwl = kv_lm_get_rwlock (lm, key, key_len);
        kv_rwl_wlock (rwl);
        {
            old_v_data = kv_ht_get (ht, &kd);
            if (old_v_data != NULL) {
                kv_ru_add (*tx_id, KV_RU_DELETE, key, key_len, NULL, 0, old_v_data->value, old_v_data->val_len);
                old_kv = kv_ht_del (ht, &kd);
            }
            else
                old_kv = (struct kv_ht_kv) { NULL, NULL };
        }
        kv_rwl_un_wlock (rwl);

        free (key);

        k_data = old_kv.key;
        old_v_data = old_kv.value;
        if (k_data != NULL && old_v_data != NULL) {
            free (k_data->key);
            free (k_data);
            free (old_v_data->value);
            free (old_v_data);

            strcpy (result, del_success);
        }
        else if (k_data == NULL && old_v_data == NULL) {
            strcpy (result, del_not_found);
        }
        else {
            // todo: handle error
        }

        if (is_single_command) {
            kv_ru_add (*tx_id, KV_RU_COMMIT, NULL, 0, NULL, 0, NULL, 0);
            kv_tx_end_transaction (*tx_id);
            *tx_id = 0;
        }
    }
    else if (is_transaction_commited (command)) {
        if (*tx_id == 0) {
            strcpy (result, transaction_not_started);
        } else {
            kv_ru_add (*tx_id, KV_RU_COMMIT, NULL, 0, NULL, 0, NULL, 0);
            kv_tx_end_transaction (*tx_id);
            *tx_id = 0;
            strcpy (result, transaction_committed);
        }
    }
    else if (is_transaction_aborted (command)) {
        if (*tx_id == 0) {
            strcpy (result, transaction_not_started);
        } else {
            kv_ru_add (*tx_id, KV_RU_ABORT, NULL, 0, NULL, 0, NULL, 0);
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

int consume_command(struct kv_ht *ht, struct kv_lm *lm, char *command, char *result, int *tx_id)
{
    size_t command_len;
    int end_of_command = find_end_of_command (command);
    if (end_of_command == -1)
        return 0;
    command_len = end_of_command - 1;

    char temp[128];  // todo: refactor: remove magic number
    memcpy (temp, command, 128);
    memset (command, 0, 128);
    memcpy (command, temp + end_of_command + 1, strlen (temp) - end_of_command - 1);
    temp[end_of_command + 1] = '\0';  // todo: perf: remove setting '\0'

    // todo: pref: command len
    run_command (ht, lm, temp, command_len, result, tx_id);
    return 1;
}

size_t str_hash_func (const void *key)
{
    struct key_data *data = (struct key_data *) key;
    return djb2 (data->key, data->key_len);
}

// todo: perf: is_equal?
int str_cmp_func (const void *key1, const void *key2)
{
    struct key_data *data1 = (struct key_data *) key1;
    struct key_data *data2 = (struct key_data *) key2;
    size_t min = data1->key_len < data2->key_len ? data1->key_len : data2->key_len;

    for (size_t i = 0; i < min; ++i) {
        if (data1->key[i] < data2->key[i])
            return -1;
        if (data1->key[i] > data2->key[i])
            return 1;
    }

    if (data1->key_len == data2->key_len)
        return 0;
    else if (data1->key_len < data2->key_len)
        return 1;
    else
        return -1;
}