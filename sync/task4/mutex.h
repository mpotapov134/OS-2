#ifndef MUTEX_H
#define MUTEX_H

#include <stdint.h>

#define MUTEX_UNLOCKED          0
#define MUTEX_LOCKED            1

struct {
    uint32_t lock;
} typedef mutex_t ;

void mutex_init(mutex_t *mutex);
void mutex_lock(mutex_t *mutex);
void mutex_unlock(mutex_t *mutex);

#endif /* MUTEX_H */
