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
//    - Barrier x
//    - BlockingLock x
//    - BoundedBuffer x 
//    - Condition 
//    - Future x
//    - Semaphore x
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
public:

    Shared(T* it) : ptr(it) {
        ptr -> ref_count.fetch_add(1);
    }
    //
    // Shared<Thing> a{};
    //
    Shared(): ptr(nullptr) {
        ptr = nullptr;
    }

    //
    // Shared<Thing> b { a };
    // Shared<Thing> c = b;
    // f(b);
    // return c;
    //
    Shared(const Shared& rhs) : ptr(rhs.ptr) {
        ptr->ref_count.fetch_add(1);
    }

    //
    // Shared<Thing> d = g();
    //
    Shared(Shared&& rhs) {
        rhs.ptr -> ref_count.add_fetch(1);
    }

    ~Shared() {
        if (ptr -> ref_count.add_fetch(-1) == 0) {
            delete ptr;
        }
    }

    // d->m();
    T* operator -> () const {
        return ptr;
    }

    // d = nullptr;
    // d = new Thing{};
    
    T* get(){
        return ptr;
    }

    // d = a;
    // d = Thing{};
    Shared<T>& operator=(const Shared<T>& rhs) {
         if (ptr -> ref_count.add_fetch(-1) == 0) {
            delete ptr;
        }
        ptr = rhs.ptr;
        ptr -> ref_count.add_fetch(1);
        return *this;
    }

    // d = g();
    Shared<T>& operator=(Shared<T>&& rhs) {
         if (ptr -> ref_count.add_fetch(-1) == 0) {
            delete ptr;
        }
        ptr = rhs.ptr;
        ptr -> ref_count.add_fetch(1);
        //MISSING();
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
        if (ptr && rhs) {
            return ptr == rhs;
        }   else
        {
            return !ptr && !rhs;
        }
    }

    bool operator!=(T* rhs) const {
        if (ptr && rhs) {
            return !(ptr == rhs);
        }   else
        {
            return !(!ptr && !rhs);
        }
    }

    // e = Shared<Thing>::make(1,2,3);
    template <typename... Args>
    static Shared<T> make(Args... args) {
        return Shared<T>{new T(args...)};
    }

};

#endif
