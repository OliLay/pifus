#ifndef PIFUS_IDENTIFIERS_H
#define PIFUS_IDENTIFIERS_H

#include "stdint.h"

typedef uint32_t app_index_t;
typedef uint32_t socket_index_t;

typedef int64_t block_offset_t;
typedef block_offset_t shmem_offset_t;

/** Uniquely identifies a socket. */
struct pifus_socket_identifier {
  app_index_t app_index;
  socket_index_t socket_index;
};

#endif
