#include "debug.h"
#include "threads.h"
#include "atomic.h"
#include "config.h"
#include "blocking_lock.h"
#include "smp.h" 

//#include "semaphore.h"

constexpr uint64_t N = 1000000;

/* Called by one CPU */
void kernelMain(void) {
    BlockingLock lock{};
    volatile uint64_t count = 0;

    auto n_threads = kConfig.totalProcs * 2;

    auto workDoneByCores = new uint32_t[kConfig.totalProcs]();
    auto workDoneByThreads = new uint32_t[n_threads]();

    for (unsigned i=0; i<n_threads; i++) {
        thread([i,workDoneByCores,workDoneByThreads,&lock,&count] {
            while (true) {
               lock.lock();
               if (count == N) stop();
               auto wc = &workDoneByCores[SMP::me()];
               *wc = (*wc) + 1;
               workDoneByThreads[i] ++;
               count ++;
               lock.unlock();
               yield();
            }
        });
    }

    while (count != N) yield();

    uint64_t sumForThreads = 0;
    for (unsigned i=0; i<n_threads; i++) {
        auto x = workDoneByThreads[i];
        Debug::printf("thread #%d did %d iterators\n",i,x);
        ASSERT(x > 0);
        sumForThreads += x;
    }
    ASSERT(sumForThreads == N);

    uint64_t sumForCores = 0;
    for (unsigned i=0; i<kConfig.totalProcs; i++) {
        auto x = workDoneByCores[i];
        Debug::printf("core #%d did %d iterations\n",i,x);
        ASSERT(x > 0);
        sumForCores += x;
    }
    ASSERT(sumForCores > 0);


    Debug::printf("*** happy times\n");
}

