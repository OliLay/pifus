#ifndef PIFUS_H
#define PIFUS_H

#include <stdint.h>

struct pifus_state {
    uint32_t app_number;
    int* app_shm_ptr;

    uint32_t highest_socket_number;
};

extern struct pifus_state state;

/**
 * @brief Initializes the interface.
 */
void pifus_initialize(void);

#endif /* PIFUS_H */
