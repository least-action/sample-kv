#include "tx_manager.h"
#include "utils.h"
#include "transaction.h"
#include "linked_list.h"
#include "transaction.h"

#include <pthread.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <memory.h>

static uint32_t transaction_id;
static pthread_mutex_t tx_lock;

// todo: add tx_list
static struct kv_ll *tx_list;
static pthread_mutex_t tx_list_lock;
FILE *tx_id_file;

static bool is_tx_equal (void *a, void *b)
{
    return kv_tx_get_id ((struct kv_tx *) a) == kv_tx_get_id ((struct kv_tx *) b);
}

static uint32_t hex_to_uint32 (char *hex)
{
    uint32_t hex_val;

    hex_val = 0;
    for (int i = 0; i < 8; ++i) {
        hex_val += from_hex (hex[i]) << ((7-i) * 4);
    }

    return hex_val;
}

static int uint32_to_hex (uint32_t tx_id, char* hex)
{
    uint32_t filter;
    uint32_t digit;    
    
    filter = 0x0000000f;
    for (int i = 0; i < 8; ++i) {
        digit = (tx_id & filter) >> (i * 4);
        hex[7 - i] = get_hex (digit);
        filter <<= 4;
    }

    return 0;
}

void kv_txm_init ()
{
    long pos;
    char last_tx_id_hex[9];

    pthread_mutex_init (&tx_lock, NULL);
    pthread_mutex_init (&tx_list_lock, NULL);
    tx_list = kv_ll_create (is_tx_equal);

    tx_id_file = fopen (KV_TX_ID_FILE, "a+");
    if (tx_id_file == NULL) {
        // todo: error handling
        exit (1);
    }
    
    fseek (tx_id_file, 0, SEEK_END);
    pos = ftell (tx_id_file);
    if (pos == 0) {
        transaction_id = 0;  // todo: from recovery file?
    } else {
        fseek (tx_id_file, -9, SEEK_END);
        if (fgets (last_tx_id_hex, 9, tx_id_file) == NULL) {
            // todo: error handling
            exit(1);
        }
        transaction_id = hex_to_uint32 (last_tx_id_hex);
    }
}

void kv_txm_destroy ()
{
    struct kv_tx *tx_elem;

    tx_elem = kv_ll_del_tail (tx_list);
    while (tx_elem != NULL) {
        kv_tx_destroy (tx_elem);
        tx_elem = kv_ll_del_tail (tx_list);
    }
    
    pthread_mutex_destroy (&tx_list_lock);
    pthread_mutex_destroy (&tx_lock);

    fclose (tx_id_file);
}


int kv_txm_lock ()
{
    pthread_mutex_lock (&tx_lock);
    return 0;
}

int kv_txm_unlock ()
{
    pthread_mutex_unlock (&tx_lock);
    return 0;
}


struct kv_tx* kv_txm_start_new_transaction ()
{
    struct kv_tx *new_tx;
    uint32_t *new_tx_id;
    char tx_id_hex[10];
    
    tx_id_hex[8] = '\n';
    tx_id_hex[9] = '\0';

    new_tx_id = malloc (sizeof (uint32_t));
    
    pthread_mutex_lock (&tx_lock);
    {
        *new_tx_id = ++transaction_id;
        new_tx = kv_tx_create (*new_tx_id);
        kv_ll_add (tx_list, new_tx);
        uint32_to_hex (transaction_id, tx_id_hex);
        fputs (tx_id_hex, tx_id_file);  // todo: bug: write multiple (maybe fork copy buffer problem)
        fflush (tx_id_file);
    }
    pthread_mutex_unlock (&tx_lock);

    return new_tx;
}

int kv_txm_end_transaction (struct kv_lm *lm, struct kv_tx *tx)
{
    kv_tx_unlock_all (lm, tx);
    pthread_mutex_lock (&tx_list_lock);
    {
        kv_ll_del (tx_list, tx);
    }
    pthread_mutex_unlock (&tx_list_lock);
    kv_tx_destroy (tx);

    return 0;
}


struct list_with_idx {
    struct kv_ongoing_tx *tx_list;
    int idx;
};

static void add_data(void *param, void *data)
{
    struct list_with_idx *lwi = (struct list_with_idx *) param;
    struct kv_tx *tx = (struct kv_tx *) data;
    struct kv_ongoing_tx tx_data;

    tx_data.tx_id = kv_tx_get_id (tx);
    tx_data.last_lsn = kv_tx_last_lsn (tx);
    lwi->tx_list[lwi->idx] = tx_data;
    ++(lwi->idx);
}

// need to use with rec file lock
size_t kv_txm_ongoing_transaction_ids (struct kv_ongoing_tx **array_start)
{
    struct kv_ongoing_tx *tx_ongoing_list;
    size_t size;
    struct list_with_idx lwi;

    size = kv_ll_size (tx_list);
    if (size == 0)
        return size;
    tx_ongoing_list = (struct kv_ongoing_tx *) malloc (size);
    lwi.tx_list = tx_ongoing_list;
    lwi.idx = 0;

    kv_ll_foreach (tx_list, add_data, &lwi);

    *array_start = tx_ongoing_list;
    return size;
}
