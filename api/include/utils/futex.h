#ifndef PIFUS_FUTEX_H
#define PIFUS_FUTEX_H

#include <sys/time.h>
#include <syscall.h>
#include <unistd.h>

struct futex_waitv {
    __u64 val;
    __u64 uaddr;
    __u32 flags;
    __u32 __reserved;
};

/**
 * futex_waitv - Wait at multiple futexes, wake on any
 * @waiters:    Array of waiters
 * @nr_waiters: Length of waiters array
 * @flags: Operation flags
 * @timo:  Optional timeout for operation
 */
// static inline int futex_waitv(volatile struct futex_waitv *waiters, unsigned
// long nr_waiters, 			      unsigned long flags, struct timespec
// *timo, clockid_t
// clockid)
//
//	return syscall(__NR_futex_waitv, waiters, nr_waiters, flags, timo,
// clockid);
//}

int futex(int* uaddr, int futex_op, int val, const struct timespec* timeout,
          int* uaddr2, int val3);

int futex_wake(uint32_t* uaddr);

int futex_wait(uint32_t* uaddr, uint32_t val);

#endif
