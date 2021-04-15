#include "debug.h"
#include "smp.h"
#include "debug.h"
#include "config.h"
#include "machine.h"

#include "threads.h"

    // Invariant: a thread will either be active or ready
    Queue<TCB,SpinLock> readyQ{};
    TCB** active;    // become a list of TCB because of multi cores
    TCB** zombie;   // To clean the memory used by all the threads.

    extern Queue<TCB,SpinLock> readyQ;
    extern TCB** active;


void threadsInit() {

    active = new TCB*[kConfig.totalProcs];
    zombie = new TCB*[kConfig.totalProcs];
    for (uint32_t i = 0; i < kConfig.totalProcs; i++){
        active[i] = new FakeTCB();
        zombie[i] = nullptr;
    }
}

extern "C" void x_contextSwitch(SaveArea* from,SaveArea* to);

// I am just curious what are the exactly purposes of namespeace,it's now kind of botherin.
void setISLock() {

    if (active[SMP::me()] -> saveArea.ISLockP != (uint32_t) nullptr) {
        ISL* iLock = (ISL*) (active[SMP::me()] -> saveArea.ISLockP);
        active[SMP::me()] -> saveArea.ISLockP = (uint32_t) nullptr;
        iLock ->unlock(true);
    }   
}

void threadClear() {

    if (zombie[SMP::me()] != nullptr) {
        delete zombie[SMP::me()];
        zombie[SMP::me()] = nullptr;
    }
    setISLock();
    TCB* me = active[SMP::me()];
    Interrupts::restore(false);
    me -> do_things();
    stop();
}

void yield() {


    bool wasDisabled = Interrupts::disable();
    TCB* prev = active[SMP::me()];
    auto next = readyQ.remove();
    if (next == nullptr) return;
    if (next ->saveArea.ready_flag == 0) {
        readyQ.add(next);
        return;
    }
    if (next ->stack == nullptr) {
        next ->stack = (uint32_t*)malloc(STACK_SIZE);
        // move %esp to give private stack
        next -> saveArea.esp = (uint32_t) &((next -> stack)[STACK_SIZE_WORDS - 1]);
        next -> stack[STACK_SIZE_WORDS - 1] = (uint32_t) threadClear;
    }
    active[SMP::me()] = next;

    // put the current one to the end of readyQ
    prev -> saveArea.ready_flag = 0;
    readyQ.add(prev);
    x_contextSwitch(&(prev -> saveArea), &(next -> saveArea)); 

    // clean memory
    if (zombie[SMP::me()] != nullptr) {
        delete zombie[SMP::me()];
        zombie[SMP::me()] = nullptr;
    }
    setISLock();
    Interrupts::restore(wasDisabled);
}

void block(ISL* lock) {
    TCB* next = readyQ.remove();
    // nothing to do, empty readyQ.
    if (next != nullptr && next ->saveArea.ready_flag == 0) {
		readyQ.add(next);
		next = nullptr;
	}
    if (next == nullptr ) {
        next = new TCBDetails([]{});
    }
    // initialize it
    if (next ->stack == nullptr) {
        next ->stack = (uint32_t*)malloc(STACK_SIZE);
        next -> saveArea.esp = (uint32_t) &((next -> stack)[STACK_SIZE_WORDS - 1]);
        next -> stack[STACK_SIZE_WORDS - 1] = (uint32_t) threadClear;
    }
    TCB* blocked = active[SMP::me()];
    active[SMP::me()] = next;
    next ->saveArea.ISLockP = (uint32_t) lock;
    x_contextSwitch(&(blocked ->saveArea), &(next ->saveArea));
    // clear previous zombie
    if (zombie[SMP::me()] != nullptr) {
        delete zombie[SMP::me()];
        zombie[SMP::me()] = nullptr;
    }
    // unlock the ISL
    setISLock();


}

void stop() {

    Interrupts::disable();
    TCB* next = nullptr;
    // try to find a ready TCB
    while (1) {
        next = readyQ.remove();
        if (next != nullptr) {
            if (next -> saveArea.ready_flag == 0) {
                readyQ.add(next);
            }
        }
        if (next != nullptr && next->saveArea.ready_flag ==1) {
            break;
        }
    }
    
    if (next ->stack == nullptr) {
        next ->stack = (uint32_t*)malloc(STACK_SIZE);
        next -> saveArea.esp = (uint32_t) &((next -> stack)[STACK_SIZE_WORDS - 1]);
        next -> stack[STACK_SIZE_WORDS - 1] = (uint32_t) threadClear;
    }

    TCB* curZombie = active[SMP::me()];
    zombie[SMP::me()] = curZombie;
    active[SMP::me()] = next;
    x_contextSwitch(&(curZombie->saveArea),&(next->saveArea));
    // do not enable it since we are done with it. 
}