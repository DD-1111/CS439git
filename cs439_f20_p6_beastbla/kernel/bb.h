#ifndef _bb_h_
#define _bb_h_

#include "debug.h"
#include "atomic.h"
#include "debug.h"
#include "queue.h"
#include "threads.h"
#include "things.h"

template <typename T>
class BoundedBuffer {
    int head;
    T* data;
    int n;
    Semaphore nEmpty;
    Semaphore nFull;
    Semophore lock;

public:
    BoundedBuffer(uint32_t n) :nEmpty(n), nFull(0), lock(1), head(0), n(0), data(new T[n]) {}

    BoundedBuffer(const BoundedBuffer&) = delete;

    void put(T t) {
        nEmpty.down();
        lock.down();
        data[(head + n) % N] = t;
        n++;
        lock.up();
        nFull.up();
    }

    T get() {
        nFull.down();
        lock.down();
        T result = data[head];
        head = (head + 1) % N;
        n--;
        lock.up();
        nEmpty.up();
        return result;
    }

#endif
