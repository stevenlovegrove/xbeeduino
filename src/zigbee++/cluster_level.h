#pragma once

#include "zcl_level.h"
#include "cluster_interface.h"
#include "cluster_attrib_utils.h"
#include "log.h"

struct cluster_level : public cluster_interface
{
    constexpr static uint16_t CLUSTOR_ID = ZCL_CLUST_LEVEL_CONTROL;
    using fn_try_update = void(void);

    cluster_level()
        : user_fn_try_update(nullptr),
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
        switch(zcl.command) {
        case ZCL_LEVEL_CMD_MOVE:
            log("ZCL_LEVEL_CMD_MOVE\n");
            break;
        case ZCL_LEVEL_CMD_STEP:
            log("ZCL_LEVEL_CMD_STEP\n");
            break;
        case ZCL_LEVEL_CMD_STOP:
            log("ZCL_LEVEL_CMD_STOP\n");
            break;
        case ZCL_LEVEL_CMD_MOVE_TO_LEVEL:
            [[fallthrough]];
        case ZCL_LEVEL_CMD_MOVE_TO_LEVEL_ON_OFF:
        {
            const uint8_t level = get_from_le_buffer<uint8_t>(zcl.zcl_payload,0);
            const uint16_t transition_time = get_from_le_buffer<uint8_t>(zcl.zcl_payload,1);
            log("Setting level to %d%% (transition time %d)\n", (int)((100.0f/255.0f)*level+0.5f), transition_time);
            current_level = level;
            break;
        }
        case ZCL_LEVEL_CMD_MOVE_ON_OFF:
            log("ZCL_LEVEL_CMD_MOVE_ON_OFF\n");
            break;
        case ZCL_LEVEL_CMD_STEP_ON_OFF:
            log("ZCL_LEVEL_CMD_STEP_ON_OFF\n");
            break;
        case ZCL_LEVEL_CMD_STOP_ON_OFF:
            log("ZCL_LEVEL_CMD_STOP_ON_OFF\n");
            break;
        default:
            log("Unrecognized command.\n");
            zcl_command_dump(&zcl);
            zcl_default_response(&zcl, ZCL_STATUS_UNSUP_GENERAL_COMMAND);
            return;
        }
        if(user_fn_try_update) user_fn_try_update();
        send_attrib_table_response(zcl, attributes);
    }

    zcl_attribute_base_t* get_attribs() override
    {
        return attributes;
    }

    fn_try_update* user_fn_try_update;
    zcl_attribute_base_t attributes[2];
    uint8_t current_level;
};
