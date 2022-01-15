#ifndef PIFUS_SHMEM_H
#define PIFUS_SHMEM_H

/**
 * @brief Tries to open a shm region with the given name.
 * 
 * @param shm_name The name of the shm region.
 * @return int the fd
 */
int shm_create_region(char* shm_name);

/**
 * @brief Maps the shared memory into the process memory space.
 *
 * @param fd The fd of the app's shared memory.
 * @param size Size of the shared mem region.
 * @return void ptr to the mapped region
 */
void* shm_map_region(int fd, size_t size);

void shm_unlink_region(char* shm_name);

#endif /* PIFUS_SHMEM_H */
