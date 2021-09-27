#pragma once

#define XBEE_PARAM(x, desc)  x,
enum class xbee_param
{
#  include "xbee_param_list.h"
    NUM_PARAMS
};
#undef XBEE_PARAM

#define XBEE_PARAM(x, desc) #x,
constexpr char xbee_param_command[][3] = {
#  include "xbee_param_list.h"
};
#undef XBEE_PARAM

#define XBEE_PARAM(x, desc) desc,
constexpr const char* xbee_param_desc[] = {
#  include "xbee_param_list.h"
};
#undef XBEE_PARAM

constexpr const char* get_param_description(xbee_param p)
{
    return xbee_param_desc[static_cast<unsigned>(p)];
}

constexpr const char* get_param_command(xbee_param p)
{
    return xbee_param_command[static_cast<unsigned>(p)];
}
