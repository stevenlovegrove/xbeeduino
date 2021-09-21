#pragma once

#include <zigbee/zcl.h>

struct cluster_interface
{
    virtual void config(zcl_command_t& zcl) = 0;
    virtual void command(zcl_command_t& zcl) = 0;
    virtual zcl_attribute_base_t* get_attribs() = 0;
};
