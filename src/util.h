#pragma once

#include <cstdio>
#include <chrono>
#include <thread>

inline void log(const char* format, ...)
{
    va_list arglist;
    va_start( arglist, format );
    vprintf( format, arglist );
    va_end( arglist );
    fflush(stdout);
}

inline void delay(unsigned long ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

// Write string, preceeded by length byte, no termination character
// Based on contribution from ST Community member - gazor
inline uint8_t* strcpy_len_prefix(uint8_t* dest, const char* str)
{
    uint8_t* d=dest+1;
    const char* s = str;
    for(; *s != 0; ++d,++s)
        *d = static_cast<uint8_t>(*s);
    *dest = s-str;
    return d;
}

bool check_xbee_result(int res)
{
    if(res >= 0) return true;

    char const* err;

    switch (res) {
    case -EPERM  : err = "Operation not permitted"; break;
    case -ENOENT : err = "No such file or directory"; break;
    case -ESRCH  : err = "No such process"; break;
    case -EINTR  : err = "Interrupted system call"; break;
    case -EIO    : err = "Input/output error"; break;
    case -ENXIO  : err = "Device not configured"; break;
    case -E2BIG  : err = "Argument list too long"; break;
    case -ENOEXEC: err = "Exec format error"; break;
    case -EBADF  : err = "Bad file descriptor"; break;
    case -ECHILD : err = "No child processes"; break;
    case -EDEADLK: err = "Resource deadlock avoided"; break;
    case -ENOMEM : err = "Cannot allocate memory"; break;
    case -EACCES : err = "Permission denied"; break;
    case -EFAULT : err = "Bad address"; break;
    case -ENOTBLK: err = "Block device required"; break;
    case -EBUSY  : err = "Device / Resource busy"; break;
    case -EEXIST : err = "File exists"; break;
    case -EXDEV  : err = "Cross-device link"; break;
    case -ENODEV : err = "Operation not supported by device"; break;
    case -ENOTDIR: err = "Not a directory"; break;
    case -EISDIR : err = "Is a directory"; break;
    case -EINVAL : err = "Invalid argument"; break;
    case -ENFILE : err = "Too many open files in system"; break;
    case -EMFILE : err = "Too many open files"; break;
    case -ENOTTY : err = "Inappropriate ioctl for device"; break;
    case -ETXTBSY: err = "Text file busy"; break;
    case -EFBIG  : err = "File too large"; break;
    case -ENOSPC : err = "No space left on device"; break;
    case -ESPIPE : err = "Illegal seek"; break;
    case -EROFS  : err = "Read-only file system"; break;
    case -EMLINK : err = "Too many links"; break;
    case -EPIPE  : err = "Broken pipe"; break;
    default: err = nullptr;
    }

    if(err) {
        log("Error (%d): %s\n", res, err);
    }else{
        log("Error (%d)\n", res);
    }

    return false;
}

#ifdef LITTLE_ENDIAN
template<typename T, typename BT>
T get_from_le_buffer(const BT* buffer, unsigned byte_offset)
{
    const uint8_t* p = (const uint8_t*)buffer + byte_offset;
    return *(T*)(p);
}

template<typename T, typename BT>
BT* put_in_le_buffer(T val, BT* buffer, unsigned byte_offset=0)
{
    const uint8_t* p = (const uint8_t*)buffer + byte_offset;
    *(T*)(p) = val;
    return buffer + sizeof(T);
}
#else
#  error "Need to check this on your platform"
#endif
