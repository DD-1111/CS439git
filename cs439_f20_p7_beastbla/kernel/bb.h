#ifndef _bb_h_
#define _bb_h_

#include "semaphore.h"
#include "atomic.h"

template <typename T>
class BoundedBuffer {
    Atomic<uint32_t> ref_count = 0;
    int head;
    int n;
    int capacity; 
    Semaphore nEmpty;
    Semaphore nFull;
    Semaphore mut;
    T* data;
public:

friend class Shared<BoundedBuffer>;
    BoundedBuffer(uint32_t N) : head(0), n(0), capacity(N), nEmpty(N), nFull(0), mut(1), data(new T[N]{}) {
    }
    ~BoundedBuffer() {
        //MISSING();
    }

    BoundedBuffer(const BoundedBuffer&) = delete;

    void put(T t) {
        nEmpty.down();
        mut.down();
        data[(head + n) % capacity] = t;
        n++;
        mut.up();
        nFull.up();
    }

    T get() {
        nFull.down();
        mut.down();
        T out = data[head];
        head = (head + 1) % capacity;
        n--;
        mut.up();
        nEmpty.up();
        return out;
    }
        
};

template <typename Out, typename Work>
Shared<BoundedBuffer<Out>> stream(uint32_t N, Work work) {
   auto s = Shared<BoundedBuffer<Out>>::make(N);
   thread([s,work]{
    work(s);
    });
   return s;
}

#endif
