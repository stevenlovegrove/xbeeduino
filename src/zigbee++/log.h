#pragma once

// Swallow all args and ignore on Arduino
#ifdef ARDUINO
#  define log(f,...)
#else

#include <cstdio>
#include <cstdarg>

inline void log(const char* format, ...)
{
    va_list arglist;
    va_start( arglist, format );
    vprintf( format, arglist );
    va_end( arglist );
    fflush(stdout);
}
#endif
