#include "spin.h"

void spinlock_init(spinlock_t *spin) {
    spin->lock = SPIN_UNLOCKED;
}

void spinlock_lock(spinlock_t *spin) {
    while (true) {
        uint32_t unlocked = SPIN_UNLOCKED;
        if (atomic_compare_exchange_strong(&spin->lock, &unlocked, SPIN_LOCKED)) {
            break;
        }
    }
}

void spinlock_unlock(spinlock_t *spin) {
    uint32_t locked = SPIN_LOCKED;
    atomic_compare_exchange_strong(&spin->lock, &locked, SPIN_UNLOCKED);
}
