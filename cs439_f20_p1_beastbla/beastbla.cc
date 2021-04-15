#include "debug.h"
#include "config.h"
#include "smp.h"
#include "critical.h"

volatile bool done[MAX_PROCS];
volatile int counters[MAX_PROCS];

void kernelMain() {
    auto me = SMP::me();

    critical_begin();

    Debug::printf("*");
    Debug::printf("*");
    Debug::printf("*");
    Debug::printf(" ");
    Debug::printf("|");
    for (unsigned i=1; i<8; i++) {
        int a = i; 
        while (--a != 0) {
            Debug::printf(" "); 
            //the number of blank spaces increases as the loop going. 
        }
        counters[me] = 1000000;
        while (-- counters[me] != 0); 
        //counters before '|'
        Debug::printf("|");
    }
    //There should be same pattern between the results given by the different kernels 
    Debug::printf("\n");

    critical_end();

    done[SMP::me()] = true;
    if (SMP::me() == 0) {
        for (unsigned i=0; i<kConfig.totalProcs; i++) {
            while (!done[i]);
        }
        Debug::shutdown();
    } else {
        while(true) asm volatile("hlt");
    }
}
