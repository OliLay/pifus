#ifndef PIFUS_FUTEX_H
#define PIFUS_FUTEX_H

#include "syscall.h"
#include "time.h"
#include "unistd.h"
#include "stdint.h"

#include "linux/futex.h"

typedef uint32_t futex_t;

/* Syscall number for futex_waitv in Kernel > 5.16. */
#define __NR_futex_waitv 449

/* Flag for 32 bit futex */
#define FUTEX_32 2

struct futex_waitv {
    uint64_t val;
    uint64_t uaddr;
    uint32_t flags;
    uint32_t __reserved;
};

/**
 * futex_waitv - Wait at multiple futexes, wake on any
 * @waiters:Array of waiters
 * @nr_waiters: Length of waiters array
 * @flags: Operation flags
 * @timo: Optional timeout for operation
 */
int futex_waitv(volatile struct futex_waitv* waiters, unsigned long nr_waiters,
                unsigned long flags, struct timespec* timo, clockid_t clockid);

int futex(uint32_t* uaddr, int futex_op, int val, const struct timespec* timeout,
          int* uaddr2, int val3);

int futex_wake(uint32_t* uaddr);

int futex_wait(uint32_t* uaddr, uint32_t val);

#endif
