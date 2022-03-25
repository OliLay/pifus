#ifndef CSV_WRITER_H
#define CSV_WRITER_H

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

char *tx_filename;
char *txed_filename;
uint64_t tx_index;
long int tx_stamps[32000000];
uint64_t txed_index;
long int txed_stamps[32000000];

void sig_handler(int signum) {
    printf("Handling signal, %li tx %li txed\n", tx_index, txed_index);
    
    FILE *file = fopen(tx_filename, "a");
    for (int i = 0; i < tx_index; i++) {
        fprintf(file, "%li\n", tx_stamps[i]);
    }
    fclose(file);

    file = fopen(txed_filename, "a");
    for (int i = 0; i < txed_index; i++) {
        fprintf(file, "%li\n", txed_stamps[i]);
    }
    fclose(file);

    exit(0);
}

void csv_writer_init() {
    signal(SIGTERM, sig_handler);
}


#endif