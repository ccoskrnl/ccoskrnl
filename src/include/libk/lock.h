#ifndef __LIBK_LOCK_H__
#define __LIBK_LOCK_H__

#include "../types.h"

typedef struct {
    volatile uint64_t lock;
} spinlock_t;

void spinlock_init(spinlock_t *s);

void spinlock_acquire(spinlock_t *s);

void spinlock_release(spinlock_t *s);

#endif