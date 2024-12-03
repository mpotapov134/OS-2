#define _GNU_SOURCE

#include <errno.h>
#include <linux/futex.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "mutex.h"

static void errExit(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

static int futex(int *uaddr, int futex_op, int val,
        const struct timespec *timeout, int *uaddr2, int val3) {
    return syscall(SYS_futex, uaddr, futex_op, val, timeout, uaddr, val3);
}

void mutex_init(mutex_t *mutex) {
    mutex->lock = MUTEX_UNLOCKED;
}

void mutex_lock(mutex_t *mutex) {
    int err;
    while (1) {
        uint32_t unlocked = MUTEX_UNLOCKED;
        if (atomic_compare_exchange_strong(&mutex->lock, &unlocked, MUTEX_LOCKED)) {
            break;
        }

        err = futex(&mutex->lock, FUTEX_WAIT, MUTEX_LOCKED, NULL, NULL, 0);
        if (err == -1 && errno != EAGAIN) {
            errExit("mutex_lock");
        }
    }
}

void mutex_unlock(mutex_t *mutex) {
    int err;
    uint32_t locked = MUTEX_LOCKED;
    if (atomic_compare_exchange_strong(&mutex->lock, &locked, MUTEX_UNLOCKED)) {
        err = futex(&mutex->lock, FUTEX_WAKE, 1, NULL, NULL, 0);
        if (err == -1) {
            errExit("mutex_unlock");
        }
    }
}
