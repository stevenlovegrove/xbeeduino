#pragma once

#include <zigbee/zcl_basic.h>
#include "cluster_interface.h"
#include "cluster_attrib_utils.h"
#include "log.h"

struct cluster_basic : public cluster_interface
{
    constexpr static uint16_t CLUSTOR_ID = ZCL_CLUST_BASIC;
    using fn_factory_reset = void(void);

    void config(zcl_command_t& zcl) override
    {
        send_configure_response(zcl, ZCL_STATUS_SUCCESS);
    }

    void command(zcl_command_t& zcl) override
    {
        switch(zcl.command) {
        case ZCL_BASIC_CMD_FACTORY_DEFAULTS:
            if(user_fn_factory_reset) user_fn_factory_reset();
            send_attrib_table_response(zcl, attributes);
            break;
        default:
            log("Unrecognized command.\n");
            zcl_command_dump(&zcl);
            zcl_default_response(&zcl, ZCL_STATUS_UNSUP_GENERAL_COMMAND);
            return;
        }
        send_attrib_table_response(zcl, attributes);
    }

    zcl_attribute_base_t* get_attribs() override
    {
        return attributes;
    }

    static void set_manufacturer_name(const char* name)
    {
        strncpy((char*)manufacturer_name, name, 32);
    }

    static void set_model_identifier(const char* name)
    {
        strncpy((char*)model_identifier, name, 32);
    }

    // These are static since they are the same for all endpoints.
    static uint8_t zcl_version;
    static uint8_t app_version;
    static uint8_t stack_version;
    static uint8_t hw_version;
    static uint8_t manufacturer_name[32];
    static uint8_t model_identifier[32];
    static uint8_t power_source;

    static fn_factory_reset* user_fn_factory_reset;
    static zcl_attribute_base_t attributes[8];
};
