#include "critical.h"
#include "debug.h"
#include "atomic.h"

static SpinLock alock;
void critical_begin() {
    alock.lock();
   
}

void critical_end() {
    alock.unlock();
}
