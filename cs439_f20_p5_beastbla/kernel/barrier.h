#ifndef _barrier_h_
#define _barrier_h_

#include "atomic.h"
#include "debug.h"
#include "stdint.h"
#include "queue.h"
#include "threads.h"

class Barrier {
    Atomic<int32_t> count; 
    
public:

    Barrier(uint32_t count) : count(count) {}

    Barrier(const Barrier&) = delete;
    
    void sync() {
        int x = count.add_fetch(-1);
        while (x != 0)
        {
            x = count.get();
            yield();
        }
        
    }

};

#endif

