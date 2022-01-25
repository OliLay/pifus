#ifndef PIFUS_CONSTANTS_H
#define PIFUS_CONSTANTS_H

/* debug */
#define PIFUS_DEBUG

/* App shmem */
#define SHM_APP_NAME_PREFIX "/app"
#define SHM_APP_SIZE 100000

/* Socket shmem */
#define SHM_SOCKET_NAME_PREFIX "-socket"
#define SHM_SOCKET_SIZE 100

/* general shmem */
#define SHM_MODE 0700

/* app */
#define MAX_APP_AMOUNT 100
#define MAX_SOCKETS_PER_APP 100

/* socket */
#define NON_EXISTENT_SOCKET_INDEX 0
#define SQUEUE_SIZE 10
#define CQUEUE_SIZE 10

/* TX */
#define TX_QUEUE_SIZE 1024
#define TX_WAIT_TIMEOUT_SEC 1
#define TX_MAX_FUTEXES_PER_THREAD 128

#endif /* PIFUS_H */
