#define _GNU_SOURCE

/* standard includes */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

/* local includes */
#include "pifus.h"
#include "pifus_ip.h"
#include "pifus_socket.h"

char *tx_filename;
char *txed_filename;

volatile size_t total_txed;
size_t warmup_count = 10000;

void parse_args(int argc, char *argv[], enum pifus_priority *prio) {
    if (argc < 2) {
        printf("At least prio is required!\n");
        exit(1);
    }

    int opt;
    while ((opt = getopt(argc, argv, "l:")) != -1) {
        switch (opt) {
            case 'l':
                *prio = str_to_prio(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s [-l]\n", argv[0]);
                exit(1);
        }
    }

    if (!asprintf(&tx_filename, "dummy_tx%s.txt", prio_str(*prio))) {
        exit(1);
    }
    if (!asprintf(&txed_filename, "dummy_txed%s.txt", prio_str(*prio))) {
        exit(1);
    }
}

void write_csv(char *filename, long value) {
    FILE *file = fopen(filename, "a");
    fprintf(file, "%li\n", value);
    fclose(file);
}

void callback_func(struct pifus_socket *socket,
                   enum pifus_operation_code op_code) {
    total_txed++;
    if (total_txed > warmup_count) {
        struct timeval tp;
        gettimeofday(&tp, NULL);
        long int us = tp.tv_sec * 1000000 + tp.tv_usec;
        write_csv(txed_filename, us);
    }

    struct pifus_operation_result result;
    pifus_socket_pop_result(socket, &result);
}

int main(int argc, char *argv[]) {
    printf("Starting pifus_dummy...\n");

    enum pifus_priority prio = PRIORITY_MEDIUM;
    parse_args(argc, argv, &prio);

    pifus_initialize(&callback_func);

    struct pifus_socket *socket = pifus_socket(PROTOCOL_TCP, prio);

    struct timeval tp;
    size_t total_tx = 0;
    while (true) {
        while (total_txed + 50 < total_tx) {
            // also schedule other threads and do not spin...
            pthread_yield();
        }

        gettimeofday(&tp, NULL);
        if (pifus_socket_nop(socket)) {
            total_tx++;

            if (total_tx > warmup_count) {
                long int us = tp.tv_sec * 1000000 + tp.tv_usec;
                write_csv(tx_filename, us);
            }
        }
    }

    pifus_exit();
    return 0;
}