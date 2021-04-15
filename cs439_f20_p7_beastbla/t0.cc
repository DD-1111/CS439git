#include "future.h"
#include "debug.h"
#include "stdint.h"

constexpr int N = 10;

volatile uint32_t thread_count = 0;

Shared<Future<uint64_t>> factorial(uint64_t n) {
    return future<uint64_t>([n] {
        thread_count ++;
        if (n == 0) {
            return uint64_t(1);
        } else {
            return factorial(n-1)->get() * n;
        }
    });
}


void kernelMain() {

    Shared<Future<int>> f = future<int>([] {
        Debug::printf("*** in thread\n");
        return 10;
    });

    Debug::printf("*** %d\n",f->get());

    for (int i=0; i<N; i++) {
        auto f = future<int>([i] {
            Debug::printf("*** in thread #%d\n",i);
            return i;
        });
        Debug::printf("*** %d\n",f->get());
    }

    uint64_t last = 1;

    for (uint32_t i=1; i<200; i++) {
        uint64_t t = factorial(i)->get();
        last = last * i;
        if (t == last) {
            Debug::printf("*** %d ok\n",i);
        } else {
            Debug::printf("*** %d mismatch\n",i);
        }
        Debug::printf("*** threads = %d\n",thread_count);
        thread_count = 0;
    }



}