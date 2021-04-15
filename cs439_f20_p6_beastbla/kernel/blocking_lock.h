#ifndef _blocking_lock_h_
#define _blocking_lock_h_

#include "debug.h"
#include "atomic.h"
#include "threads.h"
#include "things.h"

class BlockingLock {
    // Atomic<bool> taken;
    // Queue<TCB,NoLock> waitingQ;
    // ISL ISLlock;
    Semaphore sem = 1;

public:
    BlockingLock() {}

    // Can't copy
    BlockingLock(const BlockingLock&) = delete;
    BlockingLock& operator=(const BlockingLock&) = delete;

    void lock() {
        sem.down();
    }

    void unlock() {
        sem.up();
    }

    // void lock() {
    //     bool was = ISLlock.lock();
    //     if (taken) {
    //         waitingQ.add(active[SMP::me()]);
    //         block(&ISLlock);
    //     } else
    //     {
    //         taken = true;
    //         ISLlock.unlock(was);
    //     }
    // }

    // void unlock() {
    //     bool was = ISLlock.lock();
    //     auto n = waitingQ.remove();
    //     if (n != nullptr) {
    //         ISLlock.unlock(was);
    //         readyQ.add(n);
    //     } else
    //     {
    //         taken = false;
    //         ISLlock.unlock(was);
    //     }
        
    // }

};



#endif

