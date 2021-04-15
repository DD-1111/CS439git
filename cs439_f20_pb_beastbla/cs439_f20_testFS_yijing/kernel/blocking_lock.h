#ifndef _blocking_lock_h_
#define _blocking_lock_h_


#include "atomic.h"
#include "threads.h"

#include "semaphore.h"


class BlockingLock {
    Semaphore sem;
public:

    Atomic<uint32_t> ref_count {0};
    BlockingLock() : sem(1) {}

    // Can't copy
    // BlockingLock(const BlockingLock&) = delete;
    // BlockingLock& operator=(const BlockingLock&) = delete;
    BlockingLock(const BlockingLock&) = delete;


    void lock() {
        // spins as it waits for the lock to be available
        // yields to give other threads chance to make progress as it spins
        // while (taken.exchange(true)) yield();
        sem.down();
    }

    void unlock() {
        // taken.set(false);
        sem.up();
    }

    bool isMine(){
        return true;
    }

};

#endif

