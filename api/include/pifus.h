#ifndef PIFUS_H
#define PIFUS_H

#include <stdint.h>

struct pifus_app {
    uint64_t highest_socket_number;
    uint32_t highest_socket_number_futex; /* for wake-up when new socket is
                                             created */
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
