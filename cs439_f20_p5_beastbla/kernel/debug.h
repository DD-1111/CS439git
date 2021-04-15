#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <stdarg.h>
#include "io.h"
#include "stdint.h"

class Debug {
    const char* what;
    bool flag;
    static OutputStream<char> *sink;
public:
    static bool shutdown_called;
    static bool debugAll;
    static void vprintf(const char* fmt, va_list ap);
    static void printf(const char* fmt, ...);
    static void vpanic(const char* fmt, va_list ap);
    static void panic(const char* fmt, ...);
    static void missing(const char* file, int line);
    inline static void assert(bool invariant, const char* invariant_text, const char* file, int line) {
        if (!invariant) {
            panic("*** Assertion [%s] failed at %s:%d\n",invariant_text,file,line);
        }
    }
    static void vsay(const char* fmt, va_list ap);
    static void say(const char* fmt, ...);
    static void shutdown();

    static void init(OutputStream<char> *sink);

    Debug(const char* what) : what(what), flag(false) {
    }
    
    void vdebug(const char* fmt, va_list ap);
    void debug(const char* fmt, ...);

    void on() { flag = true; }
    void off() { flag = false; }
};

#define MISSING() Debug::missing(__FILE__,__LINE__)

#define ASSERT(invariant) Debug::assert(invariant,#invariant,__FILE__,__LINE__)

#define PANIC(msg) Debug::panic("panic at %s:%d [%s]",__FILE__,__LINE__,(msg))

#endif
