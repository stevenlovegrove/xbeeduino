#pragma once

#include <zigbee/zcl_onoff.h>
#include "cluster_interface.h"
#include "cluster_attrib_utils.h"
#include "log.h"

struct cluster_on_off : public cluster_interface
{
    constexpr static uint16_t CLUSTOR_ID = ZCL_CLUST_ONOFF;

    struct event {
        uint8_t endpoint;
        cluster_on_off& cluster;
        bool_t new_value;
    };

    // User notification of command to change state
    // Return true to allow update
    using fn_updated_t = bool(event);

    cluster_on_off()
        : fn_updated(nullptr),
          attributes{
            { ZCL_ONOFF_ATTR_ONOFF, ZCL_ATTRIB_FLAG_NONE, ZCL_TYPE_LOGICAL_BOOLEAN, &current_value},
            { ZCL_ATTRIBUTE_END_OF_LIST }
        } {}

    void config(zcl_command_t& zcl) override
    {
        send_configure_response(zcl, ZCL_STATUS_SUCCESS);
    }

    void command(zcl_command_t& zcl) override
    {
        event e = {zcl.envelope->dest_endpoint, *this};

        switch(zcl.command) {
        case ZCL_ONOFF_CMD_OFF:
            e.new_value = false;
            break;
        case ZCL_ONOFF_CMD_ON:
            e.new_value = true;
            break;
        case ZCL_ONOFF_CMD_TOGGLE:
            e.new_value = !current_value;
            break;
        default:
            log("Unrecognized command");
            zcl_command_dump(&zcl);
            zcl_default_response(&zcl, ZCL_STATUS_UNSUP_GENERAL_COMMAND);
            return;
        }
        // Only actually update if user function accepts change.
        if(!fn_updated || (fn_updated && fn_updated(e))) {
            current_value = e.new_value;
        }
        send_attrib_table_response(zcl, attributes);
    }

    const zcl_attribute_base_t* get_attribs() const override
    {
        return attributes;
    }


    fn_updated_t* fn_updated;
    zcl_attribute_base_t attributes[2];
    bool_t current_value;
};
