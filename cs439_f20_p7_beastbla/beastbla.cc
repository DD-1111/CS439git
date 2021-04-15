#include "debug.h"
#include "threads.h"
#include "atomic.h"
#include "shared.h"
#include "semaphore.h"

/* 
    Stress test focus on the semaphore implementation and preemption.
    Tests to see if semaphore up() properly triggers the release of all threads
    from the waitingQ without race conditions and deadlock. Also tests to see if threads are being pre-empted.

    first stage: we create an atomic counter and set it to 0. Then, we
    create 50 threads, each one incrementing counter by 1. Each thread
    continues spinning until stage two is complete.

    second stage: once the first 50 threads have finished adding one to counter,
    we create another 50 threads and add one each to counter again. While all
    this is happening, the first 50 threads CONTINUE to spin.
    Then, once counter is 100, all 100 threads complete execution and we check counter
    value is correct.
*/

void kernelMain(void) {

    Atomic<int> counter1 { 0 };
    Atomic<int> counter2 { 0 };
    Semaphore sem { 0 };

    // Create 50 threads. Each thread adds one to counter, then continues spinning
    // until second set of 50 threads have completed execution.
    for (int i=0; i<50; i++) {
        thread([&counter1] {
            counter1.add_fetch(1);
            while (counter1.get() != 100);
        });
    }

    // Spin on main thread until all threads complete.
    while (counter1.get() != 50);
    // Create second set of 50 threads, each thread adds one to counter again.
    for (int i = 0; i < 50; i++) {
        thread([&counter1] {
            counter1.add_fetch(1);
            while (counter1.get() != 100);
        });
    }

    // Wait until all 100 threads have finished execution.
    while (counter1.get() != 100);
    if (counter1.get() != 100) {
        Debug::printf("*** counter value is INCORRECT\n");
    } else {
        Debug::printf("*** counter value is CORRECT\n");
    }

// Final stage: The last thread created must be run before any other thread can progress.
// This tests makes sure that semaphores are blocking correctly and releases when conditions are met. It tests to make
// sure that all threads run to completion and are able to run.

     // Creates 9 threads that cant run until the next two threads run to completion
    for(int i = 0 ; i < 9 ; i++){
        thread([&sem, &counter2]{
            sem.down();
            counter2.add_fetch(1);
            sem.up();
            while(counter2.get() != 10){}
        });
    }

    // A thread that must run to completion in order for the top 9 threads to progress.
    // Needs the bottom thread to run before it can progress.
    thread([&sem, &counter2]{
        while(counter2.get() == 0){}
        sem.up();
    });

    // The only thread that is not dependent on any other threads. This must run to completion before any other threads can 
    // make progress.
    thread([&counter2]{
        counter2.add_fetch(1);
    });

    // Makes sure the correct value for count is set
    while(counter2.get() != 10) yield();

    if(counter2.get() == 10){
        Debug::printf("*** passed\n");
    }
    else{
        Debug::printf("*** failed\n");
    }
    // Modified tests from last week's submission, sorry for not making big contribution on our tests set.
}