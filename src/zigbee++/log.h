#pragma once

// Swallow all args and ignore on Arduino
#ifdef ARDUINO
    #include <Arduino.h>
    inline void log(const char* format, ...)
    {
        va_list arglist;
        va_start( arglist, format );
        char buffer[128];
        snprintf(buffer, 128, format, arglist);
        Serial.print(buffer);
        va_end( arglist );
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
