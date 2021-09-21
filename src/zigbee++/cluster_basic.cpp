#include "cluster_basic.h"

uint8_t cluster_basic::zcl_version = 0x00;
uint8_t cluster_basic::app_version = 0x00;
uint8_t cluster_basic::stack_version = 0x00;
uint8_t cluster_basic::hw_version = 0x00;
uint8_t cluster_basic::manufacturer_name[32] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
uint8_t cluster_basic::model_identifier[32] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
uint8_t cluster_basic::power_source = ZCL_BASIC_PS_UNKNOWN;

cluster_basic::fn_factory_reset* cluster_basic::user_fn_factory_reset = nullptr;
zcl_attribute_base_t cluster_basic::attributes[8] = {
    { ZCL_BASIC_ATTR_ZCL_VERSION, ZCL_ATTRIB_FLAG_READONLY, ZCL_TYPE_UNSIGNED_8BIT, &zcl_version},
    { ZCL_BASIC_ATTR_APP_VERSION, ZCL_ATTRIB_FLAG_READONLY, ZCL_TYPE_UNSIGNED_8BIT, &app_version},
    { ZCL_BASIC_ATTR_STACK_VERSION, ZCL_ATTRIB_FLAG_READONLY, ZCL_TYPE_UNSIGNED_8BIT, &stack_version},
    { ZCL_BASIC_ATTR_HW_VERSION, ZCL_ATTRIB_FLAG_READONLY, ZCL_TYPE_UNSIGNED_8BIT, &hw_version},
    { ZCL_BASIC_ATTR_MANUFACTURER_NAME, ZCL_ATTRIB_FLAG_READONLY, ZCL_TYPE_STRING_CHAR, &manufacturer_name},
    { ZCL_BASIC_ATTR_MODEL_IDENTIFIER, ZCL_ATTRIB_FLAG_READONLY, ZCL_TYPE_STRING_CHAR, &model_identifier},
    { ZCL_BASIC_ATTR_POWER_SOURCE, ZCL_ATTRIB_FLAG_READONLY, ZCL_TYPE_ENUM_8BIT, &power_source},
    { ZCL_ATTRIBUTE_END_OF_LIST }
};
