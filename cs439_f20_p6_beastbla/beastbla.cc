#include "debug.h"
#include "threads.h"
#include "atomic.h"
#include "bb.h"




void kernelMain(void) {
// to check preemption by making sure all threads run 
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
Debug::printf("*** passed preemption test\n");
//check bounded buffer that waits on every get() call to come back.
    BoundedBuffer<int> buffer{1};    
    int sum = 0;

    for (int i = 0; i < 20; i++) {
        thread([&, i] {
            buffer.put(i);
        });
    }
    for (int i = 0; i < 20; i++) {
        sum += buffer.get();
    }
    Debug::printf("*** sum = %d\n", sum);
}

