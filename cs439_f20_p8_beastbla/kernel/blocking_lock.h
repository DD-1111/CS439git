#ifndef _blocking_lock_h_
#define _blocking_lock_h_

#include "debug.h"
#include "semaphore.h"

class BlockingLock {
       Semaphore sem = 1;
       Atomic<uint32_t> ref_count = 0;
public:
friend class Shared<BlockingLock>;
    BlockingLock()  {
    }

    BlockingLock(const BlockingLock&) = delete;

    void lock() {
        sem.down();
    }

    void unlock() {
        sem.up();
    }

};



#endif

