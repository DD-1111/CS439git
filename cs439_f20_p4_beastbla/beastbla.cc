#include "debug.h"
#include "threads.h"
#include "atomic.h"
#include "future.h"


/*Checks calling threads recursively and check the variable iteration between threads */

#define SIZE 100 //size of array

Future<int> arr[SIZE];

void recursiveCaller(int cur) {
    if(cur<SIZE) {    
        arr[cur].set(cur);
        int nextval = cur+1;
        thread([nextval] {
                recursiveCaller(nextval);
        });
    }

}


/* Called by one CPU */
void kernelMain(void) {
    int start = 0;

    thread([start] {
            recursiveCaller(start);
    });
    int sum = 0;
    for(int i=0; i<SIZE; i++) {
       sum += arr[i].get();
    }
    Debug::printf("*** sum = %d\n", sum);
}
