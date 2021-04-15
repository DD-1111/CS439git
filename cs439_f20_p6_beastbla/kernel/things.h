#include "stdint.h"
#include "queue.h"
#include "threads.h"
#include "atomic.h"


class Semaphore {
	uint32_t count;
	Queue<TCB,NoLock> waitingQ;
	ISL ISLlock;

public:
	Semaphore(uint32_t initial) : count (initial) {}

    // adds 1 to the internal counter
	void up() { 
        // Debug::printf("%#x Semaphore::up()\n", active[SMP::me()]);
		bool was = ISLlock.lock();
		auto next = waitingQ.remove();
		if (next == nullptr) {
			count++;
		} else {
			readyQ.add(next);
		}
		ISLlock.unlock(was);
		return;	
	}

    // blocks until counter > 0 then subtracts 1
	void down() {
        // Debug::printf("%#x Semaphore::down()\n", active[SMP::me()]);
		bool was = ISLlock.lock();
		if (count == 0) {
            // unavailable, block it 
			waitingQ.add(active[SMP::me()]);
			block(&ISLlock);
			ISLlock.unlock(was);
		} else {
			count--;
			ISLlock.unlock(was);
		}
	}

};