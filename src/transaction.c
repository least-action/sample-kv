#include "transaction.h"
#include "utils.h"

#include <pthread.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

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
        lseek_with_error (begun_read_fd, -TX_DIGIT_LEN, SEEK_END);
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

struct kv_tx_id* kv_tx_ongoing_transactions ()
{
    // todo
    return NULL;
}
