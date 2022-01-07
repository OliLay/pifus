/* std includes */
#include <stdlib.h>

/* local includes */
#include "pifus_socket.h"
#include "pifus.h"

struct pifus_socket* socket(void) {
    struct pifus_socket* socket = malloc(sizeof(struct pifus_socket));

    /** 
     * TODO:
     * - Alloc shmem area (appX/socketY) with latest socket # + 1
     * - Alloc cqueue, squeue inside shmem
     * - Store notification inside app shmem for stack-side (e.g. in a lock-free queue) with ptr to socket's shmem
     * - Return internal pifus_socket ptr for further usage
    **/

   return socket;
}