#include "stdint.h"
#include "debug.h"
#include "ide.h"
#include "ext2.h"
#include "sys.h"
#include "threads.h"
#include "elf.h"
#include "future.h"
const char* initName = "/sbin/init";

namespace gheith {
    Shared<Ext2> root_fs = Shared<Ext2>::make(Shared<Ide>::make(1));
}


void kernelMain(void) {
    using namespace gheith;
    auto argv = new const char* [2];
    argv[0] = "init";
    argv[1] = nullptr;

    // create the init process
    // load /sbin/init
    // initialize a user stack with argc and argv
    // switch to user mode and start running at the entry point
    //        will most likely involve calling switchToUser
    auto root = root_fs->root;
    auto sbin = root_fs->get_node(root->find("sbin"));  
    auto init = root_fs->get_node(sbin->find("init"));

    // Debug::printf("%d\n",init->get_type());
    uint32_t f = ELF::load(init);
    uint32_t* esp = (uint32_t*) 0xefffe000;
    uint32_t argc = 1;

    esp[0] = argc;
    esp[1] = (uint32_t)argv;
    esp[2] = (uint32_t)argv[0];
    esp[3] = 0;
    memcpy((esp+4), argv[0],4);
    esp[5] = '\0';

    // Debug::printf("%d\n",*esp);
    // Debug::printf("%d\n",*(esp+1));
    // Debug::printf("%d\n",*(esp+2));
    // Debug::printf("%d\n",*(esp+3));
    // Debug::printf("%d ",*((char*)esp+16));
    //  Debug::printf("%d ",*((char*)esp+17));
    //   Debug::printf("%d ",*((char*)esp+18));
    //    Debug::printf("%d \n",*((char*)esp+19));
    // Debug::printf("%d \n",*((char*)esp+20));
   // Debug::printf("%d\n",*((char*)esp+20));
     //Debug::shutdown();
    switchToUser(f, (uint32_t) esp,0);
    //int rc = SYS::exec(initName,1,argv);
    //Debug::panic("*** rc = %d",rc);
   
}

