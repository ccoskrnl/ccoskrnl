#include "../../include/libk/lock.h"
#include "../../include/arch/lib.h"

void spinlock_init(spinlock_t *s) 
{
    s->lock = 0;  // Initialize the lock to 0 (unlocked)
}

void spinlock_acquire(spinlock_t *s)
{
    while (lock_cmpxchg(&s->lock, 1, 0) != 0) {
        // Busy-wait (spin) until the lock is acquired
    }
}

void spinlock_release(spinlock_t *s) 
{
    s->lock = 0;  // Release the lock by setting it to 0
}
