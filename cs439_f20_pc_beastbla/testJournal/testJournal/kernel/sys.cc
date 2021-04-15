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
#include "future.h"





// int write(int fd, void* buf, size_t nbytes);
int write(uint32_t* userEsp) {
    using namespace gheith;
	int fd = (int) userEsp[1];
    //Debug::printf("write fd = %d \n", fd);
	char* buf = (char*) userEsp[2];
    if ((uint32_t)buf == 0xfec00000 ||(uint32_t)buf == 0xfebfffff||(uint32_t)buf == 0xfee00000||(uint32_t)buf == 0xfedfffff||(uint32_t)buf < 0x80000000 ) {
        return -1;
    }
	size_t nbytes = (size_t) userEsp[3];
	// if (fd < 0 || fd ==0 || fd >2)return -1; 
	auto file = current() -> process ->getFile(fd);
	return file->write(buf,nbytes);
}

/* fork */
/* 0 => child, +ve => parent, -ve => error */
//  int fork();
int fork(uint32_t* userEsp, uint32_t* frame) {
    using namespace gheith;
    int returnedId = 0;
    auto child = current() ->process ->fork(returnedId);
    uint32_t e = (uint32_t) frame[0];
    thread(child,[e, userEsp] {
            switchToUser(e, (uint32_t) userEsp, 0);
        });
    return returnedId;
}

int exit(uint32_t* userEsp) {
    using namespace gheith;
	current() -> process -> output-> set(userEsp[1]); 
   // Debug::shutdown();
	stop();
	return 0;
}

// /* create semaphore */
// /* returns semaphore descriptor */
// extern int sem(uint32_t initial);
int sem(uint32_t* userEsp) {
    using namespace gheith;
    uint32_t initial = (uint32_t) userEsp[1];
    auto curPro = current() -> process;
    return curPro -> newSemaphore (initial);
}


// /* up */
// /* semaphore up */
// /* return 0 on success, -ve value on failure */
// extern int up(int id);
int upHandler(uint32_t* userEsp) {
    using namespace gheith;
    int id = (int) userEsp[1];
   // Debug::printf("uphandler id = %d\n",id);
    if (id < 0) {
        return -1;
    }
    auto sem = current() ->process ->getSemaphore(id);
    if (sem == nullptr) {
        //Debug::printf("uphandler id = %d, it is null \n",id);
        return -1;
    }
    sem->up();
    return 0;
}

// /* down */
// /* semaphore down */
// /* return 0 on success, -ve value on failure */
// extern int down(int id);
int downHandler(uint32_t* userEsp) {
    
    using namespace gheith;
    int id = (int) userEsp[1];
    //Debug::printf("downhandler id = %d\n",id);
    if (id < 0) {
        return -1;
    }
    auto sem = current() ->process ->getSemaphore(id);
    if (sem == nullptr) {
       // Debug::printf("downhandler id = %d, it is null \n",id);
        return -1;
    }
    sem->down();
    return 0;
}


// /* close */
// /* closes either a file or a semaphore or disowns a child process */
// /* return 0 on success, -ve value on failure */
// extern int close(int id);
int closeHandler(uint32_t* userEsp) {
    using namespace gheith;
	int id = (int) userEsp[1];
	if (id < 0) {
        return -1;
    }
	return current() ->process ->close(id);
}




// /* len */
// /* returns number of bytes in the file, negative indicates error or a console device */
// extern ssize_t len(int fd);
ssize_t len(uint32_t* userEsp) {
       using namespace gheith;
    int fd = (int) userEsp[1];
	if (fd < 3)  
        return -1;
    auto file = current() -> process->getFile(fd);
    if (file == nullptr) 
        return -1;
    return file->size();
}


// /* read */
// /* reads up to nbytes from file, returns number of bytes read */
// extern ssize_t read(int fd, void* buf, size_t nbyte);
ssize_t read(uint32_t* userEsp) {
    using namespace gheith;
    int fd = (int) userEsp[1];
    if (fd < 0) {
        return -1;
    }
	char* buf = (char*) userEsp[2];
	size_t nbytes = (size_t) userEsp[3];
    if ((uint32_t)buf == 0xfec00000 ||(uint32_t)buf == 0xfebfffff||(uint32_t)buf == 0xfee00000||(uint32_t)buf == 0xfedfffff ) {
        return -1;
    }
    auto curPro = current() -> process;
    auto file = curPro->getFile(fd); 
    if (file == nullptr) {
        return -1;
    }
    return file->read(buf,nbytes);
}

    

// /* seek */
// /* seek to given offset in file */
// /* returns the new offset on success, -ve value on failure */
// /* seeking in a console device is an error */
// /* seeking outside the file is not an error but might cause
//    subsequent read/write to fail */
// extern off_t seek(int fd, off_t offset);
off_t seekHandler(uint32_t* userEsp) {
    using namespace gheith;
    int fd = (int) userEsp[1];
    if (fd < 0) {
        return -1;
    }
    off_t offset = (off_t) userEsp[2];
    auto file = current() -> process -> getFile(fd);
    if (file == nullptr) {
        return -1;
    }
    return file -> seek(offset);
}

// /* shutdown */
// /* should never return */
// extern int shutdown(void);
int shutdownHandler(uint32_t* userEsp) {
	Debug::shutdown();
	return 0;
}

// /* wait */
// /* wait for a child, status filled with exit value from child */
// /* return 0 on success, -ve value on failure */
// extern int wait(int id, uint32_t *status);
int waitHandler(uint32_t* userEsp) {
     using namespace gheith;
    int id = (int) userEsp[1];
    if (id < 0) {
        return -1;
    }
    uint32_t* status = (uint32_t*) userEsp[2];
    return current() -> process->wait(id, status);    
}



uint32_t strlen(const char* str) {
	uint32_t result = 0;
	while (str[result] != '\0') {
		result += 1;
	}
	return result;
}




// /* execl */
// /* returning indicates an error */
// /* arg0 is the name of the program by convention */
// /* a nullptr indicates end of arguments */
// extern int execl(const char* path, const char* arg0, ...);
int execl(uint32_t* userEsp) {
     using namespace gheith;
    char* path = (char*) userEsp[1];
    auto root = root_fs ->root;
    Shared<Node> switchProgram = root_fs->find(root,path);
    uint32_t argc = 0;
    auto argv = (char**) & userEsp[2];
    while (argv[argc] != 0) {
        argc++;         
    }
    
    char* bufferAddress = (char*) malloc(4096);
    char* startAdd = bufferAddress;
    char* midPtr = startAdd + 2048;


	((uint32_t*) startAdd)[0] = argc;
	((uint32_t*) startAdd)[1] = (uint32_t)argv;
	

    startAdd += 8;
    for (uint32_t i = 0; i < argc; i++) {
		((uint32_t*) startAdd)[0] = (uint32_t) midPtr;
     // store the actual string argv[i] points to in midPtr
        int j = 0;
        int n = 0;
	    while (argv[i][n] != '\0') {
		    n += 1;
	    }
        while (j < n) {
            *midPtr = argv[i][j];
            j++;
            midPtr++;
        }
		*midPtr = '\0';
		midPtr += 1;
		while ((int)midPtr % 4 != 0) {
			midPtr += 1;
		}
        startAdd += 4;
	}
    // NULL
    ((uint32_t*) startAdd)[0] = 0;

    // clear the private data
    current() -> process -> clear_private();

    // load the program to run with ELF::load
    uint32_t f = ELF::load(switchProgram);

    // setting the user stack
    char* esp = (char*) (0xefffe000);
    char* espstartAdd = esp;
    char* espmidPtr = esp + (startAdd - bufferAddress);
    startAdd = bufferAddress;
    midPtr = startAdd + 2048;
   ((uint32_t*) espstartAdd)[0] = ((uint32_t*) startAdd)[0];
	espstartAdd += 4;
	startAdd += 4;
	((uint32_t*) espstartAdd)[0] = ((uint32_t) espstartAdd) + 4;
	espstartAdd += 4;
	startAdd += 4;

    for (uint32_t i = 0; i < argc; i++) {
		 ((uint32_t*) espstartAdd)[i] = (uint32_t) espmidPtr;
		uint32_t j = 0;
		while (midPtr[j] != '\0') {
			espmidPtr[j] = midPtr[j];
			j += 1;
		}
		espmidPtr[j] = '\0';
		midPtr += (j+1);
		espmidPtr += (j+1);
		while ((espmidPtr - esp) % 4 != 0) {
			espmidPtr += 1;
			midPtr += 1;
		}
	}
    switchToUser(f, (uint32_t) esp, 0);
    return 0;

} 

// /* open */
// /* opens a file, returns file descriptor, flags is ignored */
// extern int open(const char* fn, int flags);
int open(uint32_t* userEsp) {
    using namespace gheith;
    const char* fn = (const char*) userEsp[1];
    if(fn[0] == '\0') {
        return -1;
    }
    auto root = root_fs ->root;
    Shared<Node> node = root_fs->find(root,fn);
    if(node == nullptr) {
        return -1;
    }
    // Debug::printf("Checking file system: opening %s\n",fn);
    // Debug::printf("Checking file system: created openfile type is %d\n",node->get_type());
   auto curPro = current() -> process;
   Shared<File> sharedOpenfile{new OpenFile(node)};
    return curPro ->setFile (sharedOpenfile);

}

//extern int removefile(int fd, const char* path, const char* fn);
int removeHandler (uint32_t* userEsp) {
    using namespace gheith;
    int fd = (int) userEsp[1];
    const char* path = (const char*) userEsp[2];
    const char* fn = (const char*) userEsp[3];
    Debug::printf("remove %s under %s\n",fn,path);
    //auto root = root_fs -> root;
    auto removeresult = root_fs ->removeFile(fn);
    if (removeresult <0) {
        return -1;
    }
    return current() ->process ->close(fd);
}


//extern int renamefile(int fd, const char* path, const char* filename, const char* new_filename);
int renameHandler (uint32_t* userEsp) {
    using namespace gheith;
    int fd = (int) userEsp[1];
    const char* path = (const char*) userEsp[2];
    const char* fn = (const char*) userEsp[3];
    const char* new_fn = (const char*) userEsp[4];
    auto root = root_fs -> root;
    auto renameReturned = root_fs->renameFile(path,fn,new_fn);
    if (renameReturned <0) {
        return -1;
    }
    return current() ->process ->close(fd);


}

int visualizeJournalHandler() {
    using namespace gheith;
    root_fs->jsb->printJournal();
    return 1;
}

extern "C" int sysHandler(uint32_t eax, uint32_t *frame) {
    using namespace gheith;

    uint32_t *userEsp = (uint32_t*)frame[3];
   // uint32_t userPC = frame[0];

   // Debug::printf("*** syscall#%d, userEsp = %x, userPC = %x\n",eax,userEsp,userPC);

    
	switch(eax) {
        case 0:
            return exit(userEsp);
        case 1:
			return write(userEsp);
        case 2:
		 	return fork (userEsp, frame);
        case 3:
			return sem(userEsp);
        case 4:
			return upHandler(userEsp);
        case 5:
			return downHandler(userEsp);
        case 6:
			return closeHandler(userEsp);
        case 7:
			return shutdownHandler(userEsp);
        case 8:
			return waitHandler(userEsp);
        case 9:
			return execl(userEsp);
        case 10:
			return open(userEsp);
        case 12:
		    return read(userEsp);
        case 11:
		    return len(userEsp);
        case 13:
			return seekHandler(userEsp);
        case 14:
			return removeHandler(userEsp);    
        case 15:
            return renameHandler(userEsp);
        case 16:
            return visualizeJournalHandler();
            
    }

    return -1;

}   




void SYS::init(void) {
    IDT::trap(48,(uint32_t)sysHandler_,3);
}
