#include "transaction.h"
#include "utils.h"

#include <pthread.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <memory.h>


#define TX_DIGIT_LEN 8

#define BEGUN_TX_FILE "begun_tx.kvdb"
#define ENDED_TX_FILE "ended_tx.kvdb"
static int begun_tx_fd;
static int ended_tx_fd;

static int transaction_id;
static pthread_mutex_t tx_lock;

void kv_tx_init ()
{
    int last_tx_id;
    int begun_read_fd;
    off_t pos;
    char last_tx_digit[TX_DIGIT_LEN];

    begun_tx_fd = open (BEGUN_TX_FILE, O_CREAT | O_APPEND | O_WRONLY, 0644);
    ended_tx_fd = open (ENDED_TX_FILE, O_CREAT | O_APPEND | O_WRONLY, 0644);
    begun_read_fd = open (BEGUN_TX_FILE, O_RDONLY, 0644);
    pos = lseek_with_error (begun_read_fd, 0, SEEK_END);
    if (pos == 0)
        last_tx_id = 0;
    else {
        lseek_with_error (begun_read_fd, -TX_DIGIT_LEN-1, SEEK_END);
        read_with_error (begun_read_fd, last_tx_digit, TX_DIGIT_LEN);
        last_tx_id = digit_to_int (last_tx_digit, TX_DIGIT_LEN);
    }
    close (begun_read_fd);

    transaction_id = last_tx_id;
    pthread_mutex_init (&tx_lock, NULL);
}

void kv_tx_destroy ()
{
    close (begun_tx_fd);
    close (ended_tx_fd);
    pthread_mutex_destroy (&tx_lock);
}

int kv_tx_start_new_transaction ()
{
    int new_tx_id;
    ssize_t nr;
    char tx_digit[TX_DIGIT_LEN+1];

    pthread_mutex_lock (&tx_lock);
    new_tx_id = ++transaction_id;
    pthread_mutex_unlock (&tx_lock);

    int_to_digit (TX_DIGIT_LEN, new_tx_id, tx_digit);
    tx_digit[TX_DIGIT_LEN] = '\n';
    nr = write (begun_tx_fd, tx_digit, TX_DIGIT_LEN+1);
    if (nr == -1) {
        perror ("write begun tx error");
        exit (1);
    }

    return new_tx_id;
}

void kv_tx_end_transaction (int tx_id)
{
    char tx_digit[TX_DIGIT_LEN+1];
    ssize_t nr;

    int_to_digit (TX_DIGIT_LEN, tx_id, tx_digit);
    tx_digit[TX_DIGIT_LEN] = '\n';
    nr = write (ended_tx_fd, tx_digit, TX_DIGIT_LEN+1);
    if (nr == -1) {
        perror ("write begun tx error");
        exit (1);
    }
}

static bool tx_id_equal (void *tx_id_1, void *tx_id_2)
{
    char *t1 = (char *) tx_id_1;
    char *t2 = (char *) tx_id_2;

    for (int i = 0; i < TX_DIGIT_LEN; ++i) {
        if (t1[i] == t2[i])
            continue;
        return false;
    }
    return true;
}

struct kv_ll* kv_tx_ongoing_transactions ()
{
    // todo: perf linked list to hash set
    FILE *b_tx_file, *e_tx_file;
    char *key;

    char buffer[TX_DIGIT_LEN + 2];
    buffer[TX_DIGIT_LEN] = '\0';

    b_tx_file = fopen (BEGUN_TX_FILE, "a+");
    if (!b_tx_file) {
        perror ("file fopen error: BEGUN_TX_FILE");
        exit (1);
    }
    e_tx_file = fopen (ENDED_TX_FILE, "a+");
    if (!e_tx_file) {
        perror ("file fopen error: ENDED_TX_FILE");
        exit (1);
    }

    struct kv_ll *ll = kv_ll_create (tx_id_equal);

    while (fgets (buffer, TX_DIGIT_LEN + 2, b_tx_file) != NULL) {  // todo: check errno
        key = (char *) malloc (sizeof (char) * TX_DIGIT_LEN);
        memcpy (key, buffer, TX_DIGIT_LEN);
        kv_ll_add (ll, key);
    }

    while (fgets (buffer, TX_DIGIT_LEN + 2, e_tx_file) != NULL) {
        key = kv_ll_del (ll, buffer);
        free (key);
    }

    fclose (b_tx_file);
    fclose (e_tx_file);
    return ll;
}
