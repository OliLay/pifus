#define _GNU_SOURCE

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "init_sys.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"

void parse_args(int argc, char *argv[]);
void write_csv(char *filename, long value);
void do_work(void);
void thread(void *arg);
void start(void);

struct socket_wrapper {
    struct pifus_socket *socket;
    char *tx_filename;
    char *txed_filename;
};

struct socket_wrapper wrappers[1024];
uint16_t number_of_sockets;
uint16_t port;
char *reader_ip;

size_t warmup_count = 10000;

void parse_args(int argc, char *argv[]) {
    if (argc < 4) {
        printf("At least IP, port and number of sockets are required!\n");
        exit(1);
    }

    char *output_prefix = NULL;
    int opt;
    while ((opt = getopt(argc, argv, "c:o:p:")) != -1) {
        switch (opt) {
            case 'p':
                port = atoi(optarg);
                break;
            case 'c':
                number_of_sockets = atoi(optarg);
                break;
            case 'o':
                output_prefix = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s [-c, -p, -o]\n", argv[0]);
                exit(1);
        }
    }

    if (output_prefix == NULL) {
        output_prefix = "baseline_lwip_writer";
    }

    if (number_of_sockets == 0) {
        printf("Number of sockets has to be specified!\n");
        exit(1);
    }

    if (port == 0) {
        printf("Port has to be specified!\n");
        exit(1);
    }

    reader_ip = argv[optind];

    for (int i = 0; i < number_of_sockets; i++) {
        if (!asprintf(&wrappers[i].tx_filename, "%s_%i_tx.txt", output_prefix,
                      i)) {
            exit(1);
        }
        if (!asprintf(&wrappers[i].txed_filename, "%s_%i_txed.txt",
                      output_prefix, i)) {
            exit(1);
        }
    }
}

void write_csv(char *filename, long value) {
    FILE *file = fopen(filename, "a");
    fprintf(file, "%li\n", value);
    fclose(file);
}

void thread(void *arg) {
    LWIP_UNUSED_ARG(arg);

    ip_addr_t *reader_addr = (ip_addr_t *)malloc(sizeof(ip_addr_t));
    ipaddr_aton("192.168.1.201", reader_addr);

    while (true) {
        // TODO: insert socket logic here...
    }
}

void start(void) {
    sys_thread_new("app_thread", thread, NULL, DEFAULT_THREAD_STACKSIZE,
                   DEFAULT_THREAD_PRIO);
}

int main(int argc, char *argv[]) {
    parse_args(argc, argv);

    run_lwip_sys(&start, "192.168.1.200", "192.168.1.1", "255.255.255.0");

    return 0;
}
