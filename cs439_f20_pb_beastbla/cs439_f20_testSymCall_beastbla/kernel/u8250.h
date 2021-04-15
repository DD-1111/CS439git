#ifndef _U8250_H_
#define _U8250_H_

/* 8250 */

#include "io.h"
#include "file.h"

class U8250 : public OutputStream<char> {
public:
    U8250() { }
    virtual void put(char ch);
    virtual char get();
};

class U8250File : public File {
    U8250 *it;
public:
    U8250File(U8250* it) : it(it) {}
    bool isFile() override { return true; }
    bool isDirectory() override { return false; }
    off_t seek(off_t offset) { return offset; }
    off_t size() { return 0x7FFFFFFF; }
    ssize_t read(void* buffer, size_t n) {
        return -1;
    }
    ssize_t write(void* buffer, size_t n) {
        if (n == 0) return 0;
        //   Debug::printf("u8250 write called\n");
        //   Debug::printf("try to write %s\n", buffer);
        uint32_t i;
        char* position = (char*)buffer;
        for (i =0; i<n;i++) {
            it -> put(*(position));
            position++;
        }
        return n;
    }
};


#endif
