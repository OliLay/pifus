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

size_t total_txed;
size_t warmup_count = 10000;

void parse_args(int argc, char *argv[], enum pifus_priority *prio) {
    if (argc < 2) {
        printf("At least prio is required!\n");
        exit(1);
    }

    char *output_prefix = NULL;
    int opt;
    while ((opt = getopt(argc, argv, "l:o:")) != -1) {
        switch (opt) {
            case 'l':
                *prio = str_to_prio(optarg);
                break;
            case 'o':
                output_prefix = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s [-l, -o]\n", argv[0]);
                exit(1);
        }
    }

    if (output_prefix == NULL) {
        output_prefix = "dummy";
    }

    if (!asprintf(&tx_filename, "%s_tx.txt", output_prefix)) {
        exit(1);
    }
    if (!asprintf(&txed_filename, "%s_txed.txt", output_prefix)) {
        exit(1);
    }
}

void write_csv(char *filename, long value) {
    FILE *file = fopen(filename, "a");
    fprintf(file, "%li\n", value);
    fclose(file);
}

int main(int argc, char *argv[]) {
    printf("Starting pifus_dummy...\n");

    enum pifus_priority prio = PRIORITY_MEDIUM;
    parse_args(argc, argv, &prio);

    pifus_initialize(NULL);

    struct pifus_socket *socket = pifus_socket(PROTOCOL_TCP, prio);

    struct timeval tp;
    size_t total_tx = 0;
    while (true) {
        gettimeofday(&tp, NULL);
        if (pifus_socket_nop(socket)) {
            total_tx++;

            if (total_tx > warmup_count) {
                long int us = tp.tv_sec * 1000000 + tp.tv_usec;
                write_csv(tx_filename, us);
            }
        }

        struct pifus_operation_result result;

        pifus_socket_wait(socket, &result);
        total_txed++;

        if (total_txed > warmup_count) {
            struct timeval tp;
            gettimeofday(&tp, NULL);
            long int us = tp.tv_sec * 1000000 + tp.tv_usec;
            write_csv(txed_filename, us);
        }
    }

    pifus_exit();
    return 0;
}