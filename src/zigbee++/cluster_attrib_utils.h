#pragma once

#include <assert.h>
#include <zigbee/zcl.h>
#include "zcl_zha.h"

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

uint8_t* put_atrib(uint8_t* buffer, const zcl_attribute_base_t* entry, bool include_success, unsigned max_size_bytes);

uint8_t* put_attrib_requests(uint8_t* buffer, const zcl_attribute_base_t* attrib_table, int16_t requests_size, const uint16_t* requests, unsigned max_size_bytes);

uint8_t* put_attrib_table(uint8_t* buffer, const zcl_attribute_base_t* attrib_table, unsigned max_size_bytes);

int send_attrib_requests(zcl_command_t& request, const zcl_attribute_base_t* attrib_table, int16_t requests_size, const uint16_t* requests);

int send_attrib_table_response(zcl_command_t& request, const zcl_attribute_base_t* attrib_table);

int send_configure_response(zcl_command_t& request, uint8_t status);

void report_attribs(xbee_dev_t& xdev, const zcl_attribute_base_t *attr_list, const wpan_endpoint_table_entry_t *source_endpoint);
