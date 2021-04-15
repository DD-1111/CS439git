#ifndef _threads_h_
#define _threads_h_

#include "atomic.h"
#include "queue.h"
#include "heap.h"
#include "debug.h"

constexpr int STACK_SIZE = 8 * 1024; // in bytes
constexpr int STACK_SIZE_WORDS = STACK_SIZE / sizeof(uint32_t); // in 32 bit words

namespace x {

    struct SaveArea {
        uint32_t ebx;
        uint32_t esp;
        uint32_t ebp;
        uint32_t esi;
        uint32_t edi;
        uint32_t ready_flag;
    };


    struct TCB {

        TCB* next;

        SaveArea saveArea;

        uint32_t* stack = nullptr;

        virtual void do_things() = 0;
        virtual ~TCB() {}
    };

    template <typename Work>
    struct TCBDetails: public TCB {

        Work work;

        TCBDetails(Work w): work(w) {
            saveArea.ready_flag = 1;
        }

        void do_things() override {
            work();
        }

        ~TCBDetails(){
            delete stack;
        }
    };

    struct FakeTCB: public TCB {
   public:
    void do_things() {
    }
};
    // Invariant: a thread will either be active or ready
    extern Queue<TCB> readyQ;



}

extern void threadsInit();
extern void threadClear();
extern void stop();
extern void yield();

template <typename T>
void thread(T work) {
    using namespace x;

    auto tcb = new TCBDetails(work);
    readyQ.add(tcb);
}

#endif