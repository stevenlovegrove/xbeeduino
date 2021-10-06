#pragma once

#include "zcl_level.h"
#include "cluster_interface.h"
#include "cluster_attrib_utils.h"
#include "log.h"

struct cluster_level : public cluster_interface
{
    constexpr static uint16_t CLUSTOR_ID = ZCL_CLUST_LEVEL_CONTROL;

    struct event {
        uint8_t endpoint;
        cluster_level& cluster;
        uint8_t new_value;
        uint16_t transition_time;
    };

    using fn_updated_t = bool(event);

    cluster_level()
        : fn_updated(nullptr),
          attributes{
            { ZCL_LEVEL_ATTR_CURRENT_LEVEL, ZCL_ATTRIB_FLAG_NONE, ZCL_TYPE_UNSIGNED_8BIT, &current_level},
            { ZCL_ATTRIBUTE_END_OF_LIST }
        } {}

    void config(zcl_command_t& zcl) override
    {
        // TODO: Actually configure from params sent
        send_configure_response(zcl, ZCL_STATUS_SUCCESS);
    }

    void command(zcl_command_t& zcl) override
    {
        event e = {zcl.envelope->dest_endpoint, *this};

        switch(zcl.command) {
        case ZCL_LEVEL_CMD_MOVE:
            log("ZCL_LEVEL_CMD_MOVE");
            break;
        case ZCL_LEVEL_CMD_STEP:
            log("ZCL_LEVEL_CMD_STEP");
            break;
        case ZCL_LEVEL_CMD_STOP:
            log("ZCL_LEVEL_CMD_STOP");
            break;
        case ZCL_LEVEL_CMD_MOVE_TO_LEVEL:
            [[fallthrough]];
        case ZCL_LEVEL_CMD_MOVE_TO_LEVEL_ON_OFF:
        {
            e.new_value = get_from_le_buffer<uint8_t>(zcl.zcl_payload,0);
            e.transition_time = get_from_le_buffer<uint8_t>(zcl.zcl_payload,1);
            break;
        }
        case ZCL_LEVEL_CMD_MOVE_ON_OFF:
            log("ZCL_LEVEL_CMD_MOVE_ON_OFF");
            break;
        case ZCL_LEVEL_CMD_STEP_ON_OFF:
            log("ZCL_LEVEL_CMD_STEP_ON_OFF");
            break;
        case ZCL_LEVEL_CMD_STOP_ON_OFF:
            log("ZCL_LEVEL_CMD_STOP_ON_OFF");
            break;
        default:
            log("Unrecognized command");
            zcl_command_dump(&zcl);
            zcl_default_response(&zcl, ZCL_STATUS_UNSUP_GENERAL_COMMAND);
            return;
        }
        // Only actually update if user function accepts change.
        if(!fn_updated || (fn_updated && fn_updated(e))) {
            current_level = e.new_value;
        }
        send_attrib_table_response(zcl, attributes);
    }

    const zcl_attribute_base_t* get_attribs() const override
    {
        return attributes;
    }

    fn_updated_t* fn_updated;
    zcl_attribute_base_t attributes[2];
    uint8_t current_level;
};
