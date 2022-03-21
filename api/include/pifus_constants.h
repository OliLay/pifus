#ifndef PIFUS_CONSTANTS_H
#define PIFUS_CONSTANTS_H

/* debug */
//#define PIFUS_DEBUG

/* App shmem */
#define SHM_APP_NAME_PREFIX "/app"
#define SHM_APP_SIZE 1000000

/* Socket shmem */
#define SHM_SOCKET_NAME_PREFIX "-socket"
#define SHM_SOCKET_SIZE sizeof(struct pifus_socket)

/* general shmem */
#define SHM_MODE 0700

/* app */
#define MAX_APP_AMOUNT 100
#define MAX_SOCKETS_PER_APP 127

/* socket */
#define SQUEUE_SIZE 10
#define CQUEUE_SIZE 65535
#define WRITE_QUEUE_SIZE 32 // NOTE: take into consideration lwIP's MEMP_NUM_PBUF and TCP_SND_QUEUELEN
#define RECV_QUEUE_SIZE 32
#define RECV_BUFFER_SIZE 2048

/* stack */
#define MAX_DEQUEUES_PER_ITERATION 50
#define MAX_FULL_SND_BUF_ITERATIONS 64

/* TX */
#define TX_QUEUE_SIZE 512
#define TX_MAX_FUTEXES_PER_THREAD 128
#define TX_MAX_PARALLEL_OPS_PER_SOCKET 10

/* Discovery */
#define DISCOVERY_MAX_NEW_SOCKETS 10
#define DISCOVERY_WAIT_TIMEOUT_SEC 1

#endif /* PIFUS_H */
