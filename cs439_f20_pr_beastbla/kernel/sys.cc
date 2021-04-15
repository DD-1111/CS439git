#include "sys.h"
#include "stdint.h"
#include "idt.h"
#include "debug.h"
#include "threads.h"
#include "process.h"
#include "machine.h"
#include "ext2.h"
#include "elf.h"
#include "libk.h"
#include "file.h"
#include "heap.h"
#include "shared.h"
#include "kernel.h"

extern "C" int sysHandler(uint32_t eax, uint32_t *frame) {
    using namespace gheith;

    uint32_t *userEsp = (uint32_t*)frame[3];
    uint32_t userPC = frame[0];

    Debug::printf("*** syscall#%d, userEsp = %x, userPC = %x\n",eax,userEsp,userPC);

    return -1;

}   

void SYS::init(void) {
    IDT::trap(48,(uint32_t)sysHandler_,3);
}
