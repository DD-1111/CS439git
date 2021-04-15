#include "bb.h"
#include "debug.h"
#include "stdint.h"

void kernelMain() {

   auto b = stream<int>(10,[](auto out) {
       int i = 0;
       while (true) {
           out->put(i++);
       }
   });

   for (int i = 0; i<=400000; i++) {
       auto v = b->get();
       ASSERT(v == i);
       if ((v % 100000) == 0) Debug::printf("*** %d\n",v);
   }

   Debug::printf("*** happy times\n");

}
