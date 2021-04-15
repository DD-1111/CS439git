#include "debug.h"
#include "smp.h"
#include "debug.h"
#include "config.h"
#include "machine.h"

#include "threads.h"
namespace x {
    // Invariant: a thread will either be active or ready
    Queue<TCB,SpinLock> readyQ{};
    TCB** active;    // become a list of TCB because of multi cores
    TCB** zombie;   // To clean the memory used by all the threads.

    extern Queue<TCB,SpinLock> readyQ;
    extern TCB** active;
}

void threadsInit() {
    using namespace x;
    active = new TCB*[kConfig.totalProcs];
    zombie = new TCB*[kConfig.totalProcs];
    for (uint32_t i = 0; i < kConfig.totalProcs; i++){
        active[i] = new FakeTCB();
        zombie[i] = nullptr;
    }
}

extern "C" void x_contextSwitch(x::SaveArea* from,x::SaveArea* to);

// I am just curious what are the exactly purposes of namespeace,it's now kind of botherin.
void setISLock() {
    using namespace x;
    if (active[SMP::me()] -> saveArea.ISLockP != (uint32_t) nullptr) {
        InterruptSafeLock* iLock = (InterruptSafeLock*) (active[SMP::me()] -> saveArea.ISLockP);
        active[SMP::me()] -> saveArea.ISLockP = (uint32_t) nullptr;
        iLock ->unlock();
    }   
}

void threadClear() {
    using namespace x;
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
    using namespace x;

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
        next -> stack[STACK_SIZE_WORDS - 1] = (uint32_t) threadClear;
        next -> saveArea.esp = (uint32_t) &((next -> stack)[STACK_SIZE_WORDS - 1]);
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

void stop() {
    using namespace x;
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
        next -> stack[STACK_SIZE_WORDS - 1] = (uint32_t) threadClear;
        next -> saveArea.esp = (uint32_t) &((next -> stack)[STACK_SIZE_WORDS - 1]);
    }

    TCB* curZombie = active[SMP::me()];
    zombie[SMP::me()] = curZombie;
    active[SMP::me()] = next;
    x_contextSwitch(&(curZombie->saveArea),&(next->saveArea));
    // do not enable it since we are done with it. 
}