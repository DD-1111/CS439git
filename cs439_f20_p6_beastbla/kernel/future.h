#ifndef _future_h_
#define _future_h_

#include "debug.h"
#include "atomic.h"
#include "threads.h"
#include "things.h"

template <typename T>
class Future {
    Semaphore sem;
    T t{};
    // bool isReady;
    // ISL ISLlock;
    // T   t;
    // Queue<TCB,NoLock> waitingQ;
public:

    Future() : sem(0), t() {}
    T get(){
        sem.down();
        sem.up();
        return t;
    }

    void set(T v) {
        t =v;
        sem.up();
    }




    // // Can't copy a future
    // Future(const Future&) = delete;
    
    // void set(T v) {
    //     bool was = ISLlock.lock();
    //     if (isReady == true) {
    //         Debug::panic("back to future?");
    //     } else
    //     {
    //         isReady = true;
    //         t = v;
    //         auto n = waitingQ.remove();
    //         while (n != nullptr) {
    //             readyQ.add(n);
    //             n = waitingQ.remove();
    //         }
    //         ISLlock.unlock(was);
    //     }
    // }
    // T get() {
    //     bool was = ISLlock.lock();
    //     if (isReady) {
    //         ISLlock.unlock(was);
    //     } else
    //     {
    //         waitingQ.add(active[SMP::me()]);
    //         block(&ISLlock);
    //         if (!isReady) {
    //             Debug::panic("back to future fault");
    //         }
    //     }   
    //     return t;
    //  }
};

#endif
