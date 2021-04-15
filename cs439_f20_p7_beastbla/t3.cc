#include "shared.h"
#include "future.h"

class AllGone {
public:
    mutable Atomic<uint32_t> ref_count;
    Shared<Future<int>> done;
    Atomic<int> counter;

    AllGone(): ref_count(0), done(), counter(0) {
        done = Shared<Future<int>>::make();
    }

    ~AllGone() {
        done->set(counter.get());
    }

    void tick() {
        counter.fetch_add(1);
    }
};


Shared<Future<int>> f1(int n) {
    auto gone = Shared<AllGone>::make();

    for (int i=0; i<n; i++) {
        thread([gone] {
            gone->tick();
        });
    }
    return gone->done;
}

void kernelMain() {
    Debug::printf("*** %d\n",f1(20)->get());
    Debug::printf("*** %d\n",f1(30)->get());
}
