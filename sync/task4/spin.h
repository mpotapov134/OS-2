#ifndef SPIN_H
#define SPIN_H

#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>

#define SPIN_UNLOCKED           0
#define SPIN_LOCKED             1

typedef struct {
    volatile uint32_t lock;
} spinlock_t;

void spinlock_init(spinlock_t *spin);
void spinlock_lock(spinlock_t *spin);
void spinlock_unlock(spinlock_t *spin);

#endif /* SPIN_H */
