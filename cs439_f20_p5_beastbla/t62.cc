#include "debug.h"
#include "threads.h"
#include "atomic.h"
#include "random.h"
#include "barrier.h"
#include "future.h"

/*
    1.This test checks for preemption by never explitcly calling yield when threads are waiting, instead it calls pause() to ensure that yield is preemptively called
    when interrupts happen. This test also checks for correct implementation of yield() and stop() by recursively creating threads and making all others wait through
    pause() in get_factorial(). This test also makes sure that stopped threads are deleted correctly so that memory can be free for newer threads.

    2. This test makes the assumption that once a thread is no longer being used it is freed correctly such that there is enough memory left for new threads to be created, 
    otherwise you might get "out of memory" error. The test also makes the assumption that yield() is implemented correctly such that the thread that yields returns back to
    where it left off before yielding. Preemption and interrupts are handled correctly. Also assumes that future class is implemented correctly.
*/

int get_factorial(int val){
    volatile bool done = false;
    int rec;
    int result;
    if(val == 0) return 1;
    thread([&val, &done, &rec, &result]{
        rec = get_factorial(val -1);
        result = val *rec;
        done = true;
    });
    while(!done) pause();
    return result;
}

/* Called by one CPU */
void kernelMain(void) {
    Atomic<int> counter{0};
    Future<int> futureVal;
    // Checking effiency of nested threads. Threads that do nothing should not take up memory space.
    for(int i = 0; i < 10; i++){
        for(int j = 0; j < 2; j++){
            thread([&counter]{
                while(true){};// makes threads wait so that preemption happens
                stop();
                Debug::panic("*** Threads should never be here!!!\n");
            });
        }
        counter.add_fetch(1);
    }
    while(counter.get() < 10) pause(); // No need to call yield, just wait intrrupts will call yield.
    futureVal.set(get_factorial(10));
    Debug::printf("*** Sum is: %d \n", counter.get());
    Debug::printf("*** Factorial of sum: %d \n", futureVal.get());    

}
