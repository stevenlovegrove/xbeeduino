#pragma once

// Swallow all args and ignore on Arduino
#ifdef ARDUINO
    #include <Arduino.h>

    inline void log(const char* fmt)
    {
        Serial.println(fmt);
    }

    template<typename T, typename... Ts>
    void log(const char* fmt, const T& x1, Ts... xs)
    {
        while( *fmt != '\0')
        {
            if( *fmt == '%') {
                Serial.print(x1);
                log(fmt+1, xs...);
                return;
            }
            Serial.print(*fmt);
            ++fmt;
        }
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
