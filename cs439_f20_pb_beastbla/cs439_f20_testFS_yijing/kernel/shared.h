#ifndef _shared_h_
#define _shared_h_

#include "debug.h"

//
// Assumes an embedded counter design
//
// All classes/structs that support reference counting need to
// add the following to their state:
//
//         Atomic<uint32_t> ref_count;
//
// With the following restrictrions:
//    - initialized to 0
//    - accessible to "Shared"
//    - used exclusively by "Shared"
//
// All synchronization primitives need to support sharing:
//    - Barrier
//    - BlockingLock
//    - BoundedBuffer
//    - Condition
//    - Future
//    - Semaphore
//
// The "Shared" class facilitates safe sharing of objects but:
//    - It doesn't synchronize access to the object (only the ref_count)
//    - It is not thread safe itself. It is intended to be copied but
//      not shared
//    - You can take its address, you can pass it by reference, etc. But
//      it's most likely not the right thing to do.
//


template <typename T>
class Shared {
    T* ptr;

    Shared(T* it) : ptr(it) {
        add();
    }

    void add() {
        if(ptr != nullptr) {
            ptr->ref_count.add_fetch(1);
        }
    }
    
    void drop() {
        if (ptr != nullptr && ptr->ref_count.add_fetch(-1) == 0)
        {
            delete ptr;
        }
    }


public:

    // static Atomic<uint32_t> ref_count {0};

    //
    // Shared<Thing> a{};
    //
    Shared(): ptr(nullptr) {}

    //
    // Shared<Thing> b { a };
    // Shared<Thing> c = b;
    // f(b);
    // return c;
    //
    Shared(const Shared& rhs) {
        ptr = rhs.ptr;
        add();
    }

    //
    // Shared<Thing> d = g();
    //
    Shared(Shared&& rhs) {
        ptr = rhs.ptr;
        rhs.ptr = nullptr;
    }

    ~Shared() {
        //use a int to keep track how many pointers we have
        //only delete when we the the last pointer is deleted
        drop();
    }

    // d->m();
    T* operator -> () const {
        return ptr;
    }

    // d = nullptr;
    // d = new Thing{};
    Shared<T>& operator=(T* rhs) {
        if(ptr != rhs) {
            drop();
            ptr = rhs;
            add();
        }
        return *this;
    }

    // d = a;
    // d = Thing{};
    Shared<T>& operator=(const Shared<T>& rhs) {

        //Incomplete
        //what if point to null?????????????

        // drop your current reference
        if(ptr != rhs.ptr) {
            drop();
            ptr = rhs.ptr;
            add();
        }
        //point to real thing
        return *this;
    }

    // d = g();
    Shared<T>& operator=(Shared<T>&& rhs) {
        drop();
        ptr = rhs.ptr;
        rhs.ptr = nullptr;
        return *this;
    }

    // returns true iff this and rhs refer to the same object
    // Shared<Thing> x = ...
    // Shared<Thing> y = ...
    // if (x == y) { ... }
    bool operator==(const Shared<T>& rhs) const {
        return ptr == rhs.ptr;
    }

    bool operator!=(const Shared<T>& rhs) const {
        return ptr != rhs.ptr;
    }

    bool operator==(T* rhs) const {
        return ptr == rhs;
    }

    bool operator!=(T* rhs) const {
        return ptr != rhs;
    }

    // e = Shared<Thing>::make(1,2,3);
    template <typename... Args>
    static Shared<T> make(Args... args) {
        return Shared<T>{new T(args...)};
    }

};

#endif

/*
1.what is blocking mentioned in Piazza
2.some function in share.h
3.How to implement BB
4.Gheith's code doesn't restore correctly in context.s?
5.What is stream() in BB?
*/