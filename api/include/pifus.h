#ifndef PIFUS_H
#define PIFUS_H

#include "pifus_shmem.h"
#include <stdbool.h>
#include <stdint.h>

extern char *app_shm_name;
extern struct pifus_app *app_state;

typedef void (*pifus_callback)(struct pifus_socket *,
                               enum pifus_operation_code);
extern pifus_callback callback;

/**
 * @brief Initializes the interface.
 * @param callback Callback that is invoked when a socket has a new
 * entry in the cqueue. NULL if no callback should be used.
 */
void pifus_initialize(pifus_callback callback);

/**
 * @brief Shutdown the interface.
 */
void pifus_exit(void);

#endif /* PIFUS_H */
