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

    /* The weak forms ((2) and (4)) of the functions are allowed to fail spuriously,
     * that is, act as if *obj != *expected even if they are equal. When a
     * compare-and-exchange is in a loop, the weak version will yield better performance
     * on some platforms. When a weak compare-and-exchange would require a loop and a
     * strong one would not, the strong one is preferable.
     */
}

void spinlock_unlock(spinlock_t *spin) {
    uint32_t locked = SPIN_LOCKED;
    atomic_compare_exchange_strong(&spin->lock, &locked, SPIN_UNLOCKED);
}
