#ifndef PIFUS_H
#define PIFUS_H

#include <stdint.h>

struct pifus_app {
    /* Gives index of highest socket. Used as futex for wake-up when new socket
     * is created */
    uint32_t highest_socket_number;
};

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
