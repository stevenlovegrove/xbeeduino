#pragma once

#include <zigbee/zcl_bacnet.h>
#include <zigbee/zcl_identify.h>
#include <zigbee/zcl_basic.h>
#include <zigbee/zcl_basic_attributes.h>

// Not using this...
// BACNET doesn't seem necessery for minimal example.

////////////////////////////////////////////////////////////////////////////////
/// Attributes
///

using zcl_binary_input_t = zcl_binary_output_t;

zcl_binary_input_t relay1_input = { ZCL_BOOL_FALSE, ZCL_BOOL_FALSE };
zcl_binary_input_t relay2_input = { ZCL_BOOL_FALSE, ZCL_BOOL_FALSE };

uint_fast8_t relay_sync(const struct zcl_attribute_full_t FAR *entry) {
    return ZCL_STATUS_SUCCESS;
}

int relay_write(const struct zcl_attribute_full_t FAR *entry, zcl_attribute_write_rec_t *rec) {
    return zcl_decode_attribute(&entry->base, rec);
}

ZCL_BINARY_OUTPUT_VARS(relay1, "Relay1", relay1_input, relay_write, relay_sync);
ZCL_BINARY_OUTPUT_VARS(relay2, "Relay2", relay2_input, relay_write, relay_sync);

// Can add ZCL_BACNET_ENDPOINT() to endpoint table...
