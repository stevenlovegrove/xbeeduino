# include "cluster_attrib_utils.h"


uint8_t* put_atrib(uint8_t* buffer, const zcl_attribute_base_t* entry, bool include_success, unsigned max_size_bytes)
{
    assert(entry);
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
    assert(attrib_table);
    uint8_t* dest = buffer;
    for(zcl_attribute_base_t* entry = attrib_table; entry && entry->id != ZCL_ATTRIBUTE_END_OF_LIST; ++entry) {
        dest = put_atrib(dest, entry, false, max_size_bytes - (dest-buffer));
    }
    return dest;
}

int send_attrib_requests(zcl_command_t& request, zcl_attribute_base_t* attrib_table, int16_t requests_size, const uint16_t* requests)
{
    assert(attrib_table);
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

void report_attribs(xbee_dev_t& xdev, const zcl_attribute_base_t *attr_list, const wpan_endpoint_table_entry_t *source_endpoint) {
    // This is meant for device side triggered notifications on state change
    PACKED_STRUCT request {
        zcl_header_nomfg_t header;
        uint8_t payload[80];
    } request;
    int bytecount;
    int retval = 0;
    wpan_envelope_t envelope;

    wpan_envelope_create( &envelope, &xdev.wpan_dev, WPAN_IEEE_ADDR_COORDINATOR, WPAN_NET_ADDR_COORDINATOR);
    envelope.source_endpoint = source_endpoint->endpoint;
    envelope.dest_endpoint = 0x01;
    envelope.cluster_id = ZCL_CLUST_ONOFF;
    envelope.profile_id = WPAN_PROFILE_HOME_AUTOMATION;
    envelope.payload = &request;
    request.header.frame_control = ZCL_FRAME_SERVER_TO_CLIENT
            | ZCL_FRAME_TYPE_PROFILE
            | ZCL_FRAME_DISABLE_DEF_RESP;
    request.header.command = ZCL_CMD_REPORT_ATTRIB;

    bytecount = zcl_create_attribute_records(&request.payload, sizeof(request.payload), &attr_list);
    request.header.sequence = wpan_endpoint_next_trans( source_endpoint );
    envelope.length = offsetof(struct request, payload) + bytecount;
    wpan_envelope_send(&envelope);
}
