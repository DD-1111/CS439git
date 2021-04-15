#include "stdint.h"
#include "heap.h"
#include "random.h"
#include "smp.h"
#include "debug.h"

/* 
This test is a tringent test for 4-bytes aligning.

Because we have a header and a footer, a block given by malloc(x), x != 0 should at least have 12 bytes
while blocks should take a space of 12 bytes when 1<=x<=4;
they should take spaces of 16 when 5<=x<=8;
If x is increased from 1 to 8, the total spaces taken should be 4*12 + 4*16 + 0 = 112 bytes.
After a caculated number of loops, the heap should run out of the memory.
If the blocks are not aligned, there should be about 1.7k ~ 1.8k bytes left.
*/

void* get(uint32_t sz) {
    char* p = (char*)malloc(sz+1); 
    if (p != nullptr) {
        p[0] = 0x33;
        p[sz] = 0x44;
    }
    return p;
}




void kernelMain(void) {
    uint32_t me = SMP::me();
    uint32_t HEAP_SIZE = (1 << 20);
    if (me == kConfig.totalProcs-1) {
        malloc((HEAP_SIZE - 1600));
        // 1600 bytes left
        for (uint32_t i = 0; i < 14; i++) {
            // 112 bytes per loop
            for (uint32_t i = 1; i <= 8; i++) {
            malloc(i);
            }
        }
        void* p = get(99);
        if (p == nullptr) {
              Debug::printf("*** No memory left, blocks are perfect aligned to 4-bytes, test passes\n");
        } else {
                Debug::printf("*** Not perfectly aligned blocks");
                free(p);
 
        }
        Debug::shutdown();
    } else {
        while (true) {
           int sz = me + 100;
           char* p = new char[sz];
           p[sz-1] = 0x66;
           p[0] = 0x55;
           free(p);
       }
    }
}

