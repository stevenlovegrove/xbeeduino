#pragma once

#include <zigbee/zcl.h>
#include "util.h"

template<unsigned MAX_SIZE>
struct response_builder_t
{
    PACKED_STRUCT{
        zcl_header_response_t header;
        uint8_t buffer[MAX_SIZE];
    } response;

    struct sender
    {
        sender(zcl_command_t& zcl, uint8_t* header_start, uint8_t* data_start)
            : zcl(zcl), hdr(header_start), dst(data_start) {}
        ~sender() {
            const int ret = zcl_send_response(&zcl, hdr, dst - hdr);
            if( ret!=0) log("error (%s)\n", zcl_status_text);
        }
        zcl_command_t& zcl;
        uint8_t *hdr, *dst;
        sender& operator<<(uint8_t d) { *(dst++) = d; return *this; }
        sender& operator<<(uint16_t d) {
            put_in_le_buffer(d, dst);
            dst+=2; return *this;
        }
        sender& operator<<(const zcl_attribute_base_t& attrib) {
            dst += zcl_encode_attribute_value(dst, MAX_SIZE - (dst-hdr), &attrib);
            return *this;
        }
        template<typename T>
        sender& operator,(const T& d) { return (*this) << d; }
    };

    sender send(zcl_command_t& response_for, uint8_t command, bool force_profile = false) {
        response.header.command = command;
        uint8_t* hdr_start = (uint8_t *)&response + zcl_build_header(&response.header, &response_for);
        if(force_profile) {
            response.header.u.std.frame_control &= ~ZCL_FRAME_TYPE_CLUSTER;
        }
        return sender(response_for, hdr_start, &response.buffer[0]);
    }
};
