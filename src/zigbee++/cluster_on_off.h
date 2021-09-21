#pragma once

#include <zigbee/zcl_onoff.h>
#include "cluster_interface.h"
#include "cluster_attrib_utils.h"
#include "log.h"

struct cluster_on_off : public cluster_interface
{
    constexpr static uint16_t CLUSTOR_ID = ZCL_CLUST_ONOFF;
    using fn_try_update = void(void);

    cluster_on_off()
        : user_fn_try_update(nullptr),
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
        switch(zcl.command) {
        case ZCL_ONOFF_CMD_OFF:
            current_value = false;
            break;
        case ZCL_ONOFF_CMD_ON:
            current_value = true;
            break;
        case ZCL_ONOFF_CMD_TOGGLE:
            current_value = !current_value;
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
    bool_t current_value;
};
