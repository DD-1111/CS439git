#ifndef _barrier_h_
#define _barrier_h_

#include "atomic.h"
#include "semaphore.h"
#include "condition.h"
#include "debug.h"
#include "blocking_lock.h"

class Barrier {
    Atomic<uint32_t> ref_count = 0;
    Atomic<uint32_t> togo;
    Semaphore sem = 0;
public:
    Barrier(uint32_t count) : togo(count){
        
    }

    Barrier(const Barrier&) = delete;
    
    
    friend class Shared<Barrier>;
    void sync() {
        if ( togo.add_fetch(-1) == 0) {
            sem.up();
        } else
        {
            sem.down();
            sem.up();
        }
        
    }
};

class ReusableBarrier {
     Atomic<uint32_t> ref_count = 0;
    Atomic<uint32_t> count;
    BlockingLock lock {};
    BlockingLock sem {};
    const uint32_t reset;
public:
friend class Shared<ReusableBarrier>;


    ReusableBarrier(uint32_t count) : count(count), reset(count) {
        sem.lock();
    }

    ReusableBarrier(const ReusableBarrier&) = delete;

    void sync() {
        lock.lock();
        if (count.add_fetch(-1) == 0) {
            sem.unlock();
        } else
        {
            lock.unlock();
        }
        sem.lock();
        if (count.add_fetch(1) == reset) {
            lock.unlock();
        }else
        {
            sem.unlock();
        }    
    }
};
        
#endif

