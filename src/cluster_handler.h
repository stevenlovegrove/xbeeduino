#pragma once

#include <zigbee/zcl.h>
#include "response_builder.h"

uint8_t* put_atrib(uint8_t* buffer, const zcl_attribute_base_t* entry, bool include_success, unsigned max_size_bytes)
{
    uint8_t* dest = buffer;
    dest = put_in_le_buffer<uint16_t>(entry->id, dest);
    if(include_success) dest = put_in_le_buffer<uint8_t>(ZCL_STATUS_SUCCESS, dest);
    dest = put_in_le_buffer<uint8_t>(entry->type, dest);
    dest += zcl_encode_attribute_value(dest, max_size_bytes - (dest-buffer), entry);
    return dest;
}

uint8_t* put_attrib_requests(uint8_t* buffer, zcl_attribute_base_t* attrib_table, int16_t requests_size, const uint16_t* requests, unsigned max_size_bytes)
{
    uint8_t* dest = buffer;

    for(unsigned i=0; i < requests_size; ++i) {
        const uint16_t attrib = requests[i];
        const zcl_attribute_base_t* entry = zcl_find_attribute(attrib_table, attrib);
        if(entry) {
            dest = put_atrib(dest, entry, true, max_size_bytes - (dest-buffer));
        }else{
            dest = put_in_le_buffer<uint16_t>(attrib, dest);
            dest = put_in_le_buffer<uint8_t>(ZCL_STATUS_UNSUPPORTED_ATTRIBUTE, dest);
        }
    }

    return dest;
}

uint8_t* put_attrib_table(uint8_t* buffer, zcl_attribute_base_t* attrib_table, unsigned max_size_bytes)
{
    uint8_t* dest = buffer;
    for(zcl_attribute_base_t* entry = attrib_table; entry && entry->id != ZCL_ATTRIBUTE_END_OF_LIST; ++entry) {
        dest = put_atrib(dest, entry, false, max_size_bytes - (dest-buffer));
    }
    return dest;
}

int send_attrib_requests(zcl_command_t& request, zcl_attribute_base_t* attrib_table, int16_t requests_size, const uint16_t* requests)
{
    const unsigned MAX_SIZE=40;
    PACKED_STRUCT{
        zcl_header_response_t header;
        uint8_t buffer[MAX_SIZE];
    } response;

    response.header.command = ZCL_CMD_READ_ATTRIB_RESP;
    uint8_t* hdr_start = (uint8_t *)&response + zcl_build_header(&response.header, &request);
    uint8_t* end = put_attrib_requests(&response.buffer[0], attrib_table, requests_size, requests, MAX_SIZE);
    return zcl_send_response(&request, hdr_start, end - hdr_start);
}

int send_attrib_table_response(zcl_command_t& request, zcl_attribute_base_t* attrib_table)
{
    const unsigned MAX_SIZE=40;
    PACKED_STRUCT{
        zcl_header_response_t header;
        uint8_t buffer[MAX_SIZE];
    } response;

    response.header.command = ZCL_CMD_REPORT_ATTRIB;
    uint8_t* hdr_start = (uint8_t *)&response + zcl_build_header(&response.header, &request);
    response.header.u.std.frame_control &= ~ZCL_FRAME_TYPE_CLUSTER;
    uint8_t* dest = &response.buffer[0];
    dest = put_attrib_table(dest, attrib_table, MAX_SIZE);
    return zcl_send_response(&request, hdr_start, dest - hdr_start);
}

int send_configure_response(zcl_command_t& request, uint8_t status)
{
    const unsigned MAX_SIZE=40;
    PACKED_STRUCT{
        zcl_header_response_t header;
        uint8_t buffer[MAX_SIZE];
    } response;

    response.header.command = ZCL_CMD_CONFIGURE_REPORT_RESP;
    uint8_t* hdr_start = (uint8_t *)&response + zcl_build_header(&response.header, &request);
    uint8_t* dest = &response.buffer[0];
    dest = put_in_le_buffer<uint8_t>(status, &response.buffer[0]);
    dest = put_in_le_buffer<uint8_t>(ZCL_DIRECTION_SEND, &response.buffer[0]);
    dest = put_in_le_buffer<uint8_t>(0x00, &response.buffer[0]); // what do these mean?
    dest = put_in_le_buffer<uint8_t>(0x00, &response.buffer[0]); // what do these mean?
    return zcl_send_response(&request, hdr_start, dest - hdr_start);
}

struct cluster_handler
{
public:
    using config_function = void(zcl_command_t& request);
    using attrib_read_function = void(zcl_command_t& request);
    using command_function = void(zcl_command_t& request);

    cluster_handler(uint16_t cluster_id, config_function* cfg_fn, attrib_read_function* atr_fn, command_function* cmd_fn)
        : cluster_id(cluster_id), cfg_fn(cfg_fn), atr_fn(atr_fn), cmd_fn(cmd_fn)
    {
    }

    wpan_cluster_table_entry_t entry()
    {
        return {cluster_id, &rx_handler, this, WPAN_CLUST_FLAG_INPUT};
    }

private:
    uint16_t cluster_id;
    config_function* cfg_fn;
    command_function* cmd_fn;
    attrib_read_function* atr_fn;

    int rx_handler(const wpan_envelope_t *envelope)
    {
        zcl_command_t zcl;
        if(zcl_command_build(&zcl, envelope, nullptr) == 0) {
            const uint8_t endpoint = zcl.envelope->dest_endpoint;
            // Handle all the commands
            if(ZCL_CMD_MATCH(&zcl.frame_control, GENERAL, CLIENT_TO_SERVER, PROFILE)) {
                if(zcl.command == ZCL_CMD_READ_ATTRIB) {
                    log("Handling response for attrib read\n");
                    assert(zcl.length%2 ==0);
                    if(atr_fn) {
                        atr_fn(zcl);
                    }else{
                        zcl_default_response(&zcl, ZCL_STATUS_UNSUPPORTED_ATTRIBUTE);
                    }
                    return 0;
                } else if(zcl.command == ZCL_CMD_CONFIGURE_REPORT) {
                    log("Handling response for configure report command\n");
                    if(cfg_fn) {
                        cfg_fn(zcl);
                    }else{
                        send_configure_response(zcl, ZCL_STATUS_SUCCESS);
                    }
                    return 0;
                } else if(zcl.command == ZCL_CMD_DEFAULT_RESP) {
                    log("Got default response\n");
                    return zcl_default_response(&zcl, ZCL_STATUS_SUCCESS);
                }else {
                    log("Unhandled Profile command received\n");
                    zcl_command_dump(&zcl);
                    return zcl_default_response(&zcl, ZCL_STATUS_UNSUP_GENERAL_COMMAND);
                }
            }else if(ZCL_CMD_MATCH(&zcl.frame_control, GENERAL, CLIENT_TO_SERVER, CLUSTER)) {
                if(cmd_fn) {
                    cmd_fn(zcl);
                }else{
                    zcl_command_dump(&zcl);
                    zcl_default_response(&zcl, ZCL_STATUS_UNSUP_GENERAL_COMMAND);
                }
                return 0;
            }else{
                log("Unrecognized cluster message.\n");
                zcl_command_dump(&zcl);
                return zcl_default_response(&zcl, ZCL_STATUS_UNSUP_GENERAL_COMMAND);
            }
        } else {
            log("Unable to build ZCL Structure.\n");
            return 0;
        }
    }

    static int rx_handler(const wpan_envelope_t *envelope, void* context)
    {
        cluster_handler* self = static_cast<cluster_handler*>(context);
        return self->rx_handler(envelope);
    }
};
