// Constants and expected responses detailed here:
// https://zigbeealliance.org/wp-content/uploads/2019/12/07-5123-06-zigbee-cluster-library-specification.pdf

#pragma once

// Attributes

#define ZCL_LEVEL_ATTR_CURRENT_LEVEL          0X0000
#define ZCL_LEVEL_ATTR_REMAINING_TIME         0X0001
#define ZCL_LEVEL_ATTR_ON_OFF_TRANSITION_TIME 0X0010
#define ZCL_LEVEL_ATTR_ON_LEVEL               0X0011
#define ZCL_LEVEL_ATTR_ON_TRANSITION_TIME     0X0012
#define ZCL_LEVEL_ATTR_OFF_TRANSITION_TIME    0X0013
#define ZCL_LEVEL_ATTR_DEFAULT_MOVE_RATE      0X0014

// Attributes types
#define ZCL_LEVEL_ATTR_CURRENT_LEVEL_TYPE          ZCL_TYPE_UNSIGNED_8BIT
#define ZCL_LEVEL_ATTR_REMAINING_TIME_TYPE         ZCL_TYPE_UNSIGNED_16BIT
#define ZCL_LEVEL_ATTR_ON_OFF_TRANSITION_TIME_TYPE ZCL_TYPE_UNSIGNED_16BIT
#define ZCL_LEVEL_ATTR_ON_LEVEL_TYPE               ZCL_TYPE_UNSIGNED_8BIT
#define ZCL_LEVEL_ATTR_ON_TRANSITION_TIME_TYPE     ZCL_TYPE_UNSIGNED_16BIT
#define ZCL_LEVEL_ATTR_OFF_TRANSITION_TIME_TYPE    ZCL_TYPE_UNSIGNED_16BIT
#define ZCL_LEVEL_ATTR_DEFAULT_MOVE_RATE_TYPE      ZCL_TYPE_UNSIGNED_16BIT

// Commands
#define ZCL_LEVEL_CMD_MOVE_TO_LEVEL           0x00
#define ZCL_LEVEL_CMD_MOVE                    0x01
#define ZCL_LEVEL_CMD_STEP                    0x02
#define ZCL_LEVEL_CMD_STOP                    0x03
#define ZCL_LEVEL_CMD_MOVE_TO_LEVEL_ON_OFF    0x04
#define ZCL_LEVEL_CMD_MOVE_ON_OFF             0x05
#define ZCL_LEVEL_CMD_STEP_ON_OFF             0x06
#define ZCL_LEVEL_CMD_STOP_ON_OFF             0x07 // alias for ZCL_LEVEL_CMD_STOP

