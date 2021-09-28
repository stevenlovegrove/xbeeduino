#pragma once

#ifdef ARDUINO
inline void log(const char* format, ...)
{
    // TODO: Replace with something sane.
}
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
