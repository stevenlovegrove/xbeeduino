#pragma once

#include <stdint.h>

// Write string, preceeded by length byte, no termination character
// Based on contribution from ST Community member - gazor
uint8_t* strcpy_len_prefix(uint8_t* dest, const char* str);

// Check return code from xbee API, printing any errors
// return true on success code
bool check_xbee_result(int res);

#ifndef ARDUINO
// Mirror Arduino delay method for porting later.
void delay(unsigned long ms);
#endif
