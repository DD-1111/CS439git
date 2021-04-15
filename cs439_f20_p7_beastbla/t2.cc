#include "debug.h"
#include "atomic.h"
#include "threads.h"
#include "shared.h"
#include "future.h"

struct Thing {
    Atomic<uint32_t> ref_count{0};
    Shared<Future<int>> f = Shared<Future<int>>::make();

    ~Thing() {
        Debug::printf("*** gone\n");
        f->set(42);
    }
};

void kernelMain() {

    auto thing = Shared<Thing>::make();

    thread([thing] {
        Debug::printf("in thread\n");
    });

    auto f = thing->f;

    thing = nullptr;

    Debug::printf("*** main %d\n",f->get());

}