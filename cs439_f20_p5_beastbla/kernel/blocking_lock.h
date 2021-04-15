#ifndef _blocking_lock_h_
#define _blocking_lock_h_

#include "debug.h"
#include "atomic.h"
#include "threads.h"
class BlockingLock {
    Atomic<bool> taken;
public:
    BlockingLock() : taken(false) {}

    // Can't copy
    BlockingLock(const BlockingLock&) = delete;
    BlockingLock& operator=(const BlockingLock&) = delete;

    void lock() {
        // spins as it waits for the lock to be available
        // yields to give other threads chance to make progress as it spins
        while (taken.exchange(true)) yield();
    }

    void unlock() {
        taken.set(false);
    }

};



#endif

