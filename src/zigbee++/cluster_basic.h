#pragma once

#include <zigbee/zcl_basic.h>
#include "cluster_interface.h"
#include "cluster_attrib_utils.h"
#include "log.h"

struct cluster_basic : public cluster_interface
{
    constexpr static uint16_t CLUSTOR_ID = ZCL_CLUST_BASIC;
    using fn_factory_reset = void(void);

    cluster_basic()
        : zcl_version(0x00), app_version(0x00), stack_version(0x00), hw_version(0x00),
          manufacturer_name("\0"), model_identifier("\0"), power_source(ZCL_BASIC_PS_UNKNOWN),
          user_fn_factory_reset(nullptr),
          attributes{
            { ZCL_BASIC_ATTR_ZCL_VERSION, ZCL_ATTRIB_FLAG_READONLY, ZCL_TYPE_UNSIGNED_8BIT, &zcl_version},
            { ZCL_BASIC_ATTR_APP_VERSION, ZCL_ATTRIB_FLAG_READONLY, ZCL_TYPE_UNSIGNED_8BIT, &app_version},
            { ZCL_BASIC_ATTR_STACK_VERSION, ZCL_ATTRIB_FLAG_READONLY, ZCL_TYPE_UNSIGNED_8BIT, &stack_version},
            { ZCL_BASIC_ATTR_HW_VERSION, ZCL_ATTRIB_FLAG_READONLY, ZCL_TYPE_UNSIGNED_8BIT, &hw_version},
            { ZCL_BASIC_ATTR_MANUFACTURER_NAME, ZCL_ATTRIB_FLAG_READONLY, ZCL_TYPE_STRING_CHAR, &manufacturer_name},
            { ZCL_BASIC_ATTR_MODEL_IDENTIFIER, ZCL_ATTRIB_FLAG_READONLY, ZCL_TYPE_STRING_CHAR, &model_identifier},
            { ZCL_BASIC_ATTR_POWER_SOURCE, ZCL_ATTRIB_FLAG_READONLY, ZCL_TYPE_ENUM_8BIT, &power_source},
            { ZCL_ATTRIBUTE_END_OF_LIST }
        } {}

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

    uint8_t zcl_version;
    uint8_t app_version;
    uint8_t stack_version;
    uint8_t hw_version;
    uint8_t manufacturer_name[32];
    uint8_t model_identifier[32];
    uint8_t power_source;

    fn_factory_reset* user_fn_factory_reset;
    zcl_attribute_base_t attributes[8];
};
