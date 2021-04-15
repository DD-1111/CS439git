#include "libc.h"



// int main(int argc, char** argv) {
//     printf("*** Beforew: \n");
//     int fd = open("/data/data.txt",0);
//     printf("*** fd = %d\n", fd);
//     cp(fd,1);
//     seek(fd,4);
//     write(fd,"beastblackGa is here",28);
//     printf("After: \n");
//     seek(fd,0);
//     cp(fd,1);
//      printf("Renaming file: \n");
//     renamefile(fd,"/data","data.txt","dete.txt");
//     int fd2 = open("/data/dete.txt",0);
//     cp(fd2,1);
//     // removefile(fd2, "/data/dete.txt","data.txt");
//     // int fd3 = open("/data/dete.txt",0);
//     // cp(fd3,1);
//     shutdown();
//     return 0;
// }
//dwdwdw

// int tryOpenCp(const char *fn, int display) {
//     int fd = open (fn,0);
//     if (fd > 0) {
//         printf("*** %s found\n",fn);
//         if(display) {
//             cp (fd,1);
//         }
//     } else
//     {
//         printf("*** open sysCall failed\n");
//     }
//     return fd;
// }

int main(int argc, char** argv) {
    printf("\n");

    printf("opening data.txt: \n");
    printf("\n");
    int fd = open("/data/data.txt", 0);
    cp(fd,1);
    printf("\n");

    printf("writing to data.txt...\n");
     printf("\n");
     seek(fd, 4);
    write(fd, "Data has changed, do panic !",29 );

    seek(fd, 0);
    printf("data.txt after write(): \n");
    printf("\n");
    cp(fd,1);
    printf("\n");
    printf("\n");
    
    printf("renaming data.txt to dete.txt... \n");
    printf("\n");
    renamefile(fd, "/data/", "data.txt", "dete.txt");

    printf("opening dete.txt: \n");
    printf("\n");
    fd = open("/data/dete.txt", 0);
    if(fd > 0) {
        cp(fd,0);
        printf("\n");
    }
    
    printf("\n");
    printf("removing dete.txt \n");
    removefile(fd, "/data/", "dete.txt");
  
    printf("\n");
    printf("trying to open dete.txt \n");
    fd = open("/dete.txt", 0);

    if (fd> 0) {
        cp(fd,1);
    } else
    {
        printf("*** That file is not here ;(\n");
    }
    
    shutdown();
    return 0;
}

