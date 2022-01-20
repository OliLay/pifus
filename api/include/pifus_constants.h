#ifndef PIFUS_CONSTANTS_H
#define PIFUS_CONSTANTS_H

/* App shmem */
#define SHM_APP_NAME_PREFIX "/app"
#define SHM_APP_SIZE 100000

/* Socket shmem */
#define SHM_SOCKET_NAME_PREFIX "-socket"
#define SHM_SOCKET_SIZE 100

/* general shmem */
#define SHM_MODE 0700
#define MAX_FUTEXES_PER_TX_THREAD 128

/* app */
#define MAX_APP_AMOUNT 100
#define MAX_SOCKETS_PER_APP 100

/* socket */
#define NON_EXISTENT_SOCKET_INDEX 0

#define SQUEUE_SIZE 10
#define CQUEUE_SIZE 10


#endif /* PIFUS_H */
