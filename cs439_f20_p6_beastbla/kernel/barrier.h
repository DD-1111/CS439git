#ifndef _barrier_h_
#define _barrier_h_

#include "debug.h"
#include "atomic.h"
#include "debug.h"
#include "stdint.h"
#include "queue.h"
#include "threads.h"
#include "things.h"

class Barrier {
    Atomic<int32_t> count;
    // ISL ISLlock;
    // Queue<TCB,NoLock> waitingQ;
    Semaphore sem;
public:

   Barrier(uint32_t count) : count(count), sem(0) {}


    void sync(){
        if (count.add_fetch(-1) == 0) {
            sem.up();
        } else
        {
            sem.down();
            sem.up();
        }
        
    }
    // Barrier(const Barrier&) = delete;
    
    // void sync() {
    //      int x = count.add_fetch(-1);
    //      if(x < 0) Debug::panic("negative barrier count");
    //      if (x == 0) {
    //         bool was = ISLlock.lock();
    //         TCB* n = waitingQ.remove();
    //         while (n != nullptr) {
    //             readyQ.add(n);
    //             n = waitingQ.remove();
    //         }
    //         ISLlock.unlock(was);
    //      } else
    //      {
    //         ISLlock.lock();
    //         waitingQ.add(active[SMP::me()]);
    //         block(&ISLlock);
    //      }
         
    // }
};

class ReusableBarrier {
public:

    ReusableBarrier(uint32_t count) { MISSING(); }

    ReusableBarrier(const ReusableBarrier&) = delete;

    void sync() { MISSING(); }
};
        
#endif
