#include "debug.h"
#include "threads.h"
#include "atomic.h"
// test for interrupting infinite-loop-threads  
void kernelMain(void) {
    Atomic<int> waitingFor { 0 };
    int count = 0;
    while(count < 3) {
        for (int i=0; i<10; i++) {
            thread([&waitingFor] {
            while(true);
            });
        }
        thread([&waitingFor] {
            yield();
            waitingFor.add_fetch(1);
        });
        count += 1;
    }
    while (waitingFor.get() != 3);
    Debug::printf("*** passed\n");
}
