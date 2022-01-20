#include "utils/futex.h"

int futex_waitv(volatile struct futex_waitv* waiters, unsigned long nr_waiters,
                unsigned long flags, struct timespec* timo, clockid_t clockid) {
    return syscall(__NR_futex_waitv, waiters, nr_waiters, flags, timo, clockid);
}

int futex(uint32_t* uaddr, int futex_op, int val, const struct timespec* timeout,
          int* uaddr2, int val3) {
    return syscall(SYS_futex, uaddr, futex_op, val, timeout, uaddr2, val3);
}

int futex_wait(uint32_t* uaddr, uint32_t val) {
    return futex(uaddr, FUTEX_WAIT, val, NULL, NULL, 0);
}

int futex_wake(uint32_t* uaddr) {
    return futex(uaddr, FUTEX_WAKE, 1, NULL, NULL, 0);
}