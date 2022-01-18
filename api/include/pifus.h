#ifndef PIFUS_H
#define PIFUS_H

#include <stdint.h>
#include "pifus_shmem.h"

extern char* app_shm_name;
extern struct pifus_app* app_state;

/**
 * @brief Initializes the interface.
 */
void pifus_initialize(void);

/**
 * @brief Shutdown the interface.
 */
void pifus_exit(void);

#endif /* PIFUS_H */
