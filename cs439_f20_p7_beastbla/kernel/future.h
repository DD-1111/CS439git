#ifndef _future_h_
#define _future_h_

#include "atomic.h"
#include "semaphore.h"
#include "threads.h"
#include "shared.h"


template <typename T>
class Future {
    Atomic<uint32_t> ref_count = 0;
    Semaphore sem;
    T t;
    friend class Shared<Future>;
public:

    Future() : sem(0), t() {}
    // Can't copy a future
    Future(const Future&) = delete;
    
    T get(){
        sem.down();
        sem.up();
        return t;
    }

    void set(T v) {
        t = v;
        sem.up();
    }

};

template <typename Out, typename T>
Shared<Future<Out>> future(T work) {
    auto f = Shared<Future<Out>>::make();
    thread([f,work] {
        f->set(work());
    });
    return f;
}

#endif

