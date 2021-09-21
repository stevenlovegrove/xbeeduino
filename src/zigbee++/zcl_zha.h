#pragma once

#include <xbee/discovery.h>
#include <xbee/atcmd.h>
#include <zigbee/zcl.h>
#include <zigbee/zcl_client.h>
#include "util.h"

////////////////////////////////////////////////////////////////////////////////
/// ZIGBEE HOME AUTOMATION PUBLIC APPLICATION PROFILE
/// https://d1.amobbs.com/bbs_upload782111/files_46/ourdev_678097G9TJIF.pdf
///

// Profile constant:
#define WPAN_PROFILE_HOME_AUTOMATION 0x104

////////////////////////////////////////////////////////////////////////////////
/// Device Constants (Zigbee Home Automation Public Application Profile, page 35)
///

// Generic
#define ZHA_DEVICE_ON_OFF_SWITCH 0X0000
#define ZHA_DEVICE_LEVEL_CONTROL_SWITCH 0X0001
#define ZHA_DEVICE_ON_OFF_OUTPUT 0X0002
#define ZHA_DEVICE_LEVEL_CONTROLLABLE_OUTPUT 0X0003
#define ZHA_DEVICE_SCENE_SELECTOR 0X0004
#define ZHA_DEVICE_CONFIGURATION_TOOL 0X0005
#define ZHA_DEVICE_REMOTE_CONTROL 0X0006
#define ZHA_DEVICE_COMBINED_INTERFACE 0X0007
#define ZHA_DEVICE_RANGE_EXTENDER 0X0008
#define ZHA_DEVICE_MAINS_POWER_OUTLET 0X0009
#define ZHA_DEVICE_DOOR_LOCK 0X000A
#define ZHA_DEVICE_DOOR_LOCK_CONTROLLER 0X000B
#define ZHA_DEVICE_SIMPLE_SENSOR 0X000C

// Lighting
#define ZHA_DEVICE_ON_OFF_LIGHT 0X0100
#define ZHA_DEVICE_DIMMABLE_LIGHT 0X0101
#define ZHA_DEVICE_COLOR_DIMMABLE_LIGHT 0X0102
#define ZHA_DEVICE_ON_OFF_LIGHT_SWITCH 0X0103
#define ZHA_DEVICE_DIMMER_SWITCH 0X0104
#define ZHA_DEVICE_COLOR_DIMMER_SWITCH 0X0105
#define ZHA_DEVICE_LIGHT_SENSOR 0X0106
#define ZHA_DEVICE_OCCUPANCY_SENSOR 0X0107

// Closures
#define ZHA_DEVICE_SHADE 0X0200
#define ZHA_DEVICE_SHADE_CONTROLLER 0X0201
#define ZHA_DEVICE_WINDOW_COVERING_DEVICE 0X0202
#define ZHA_DEVICE_WINDOW_COVERING_CONTROLLER 0X0203

// HVAC
#define ZHA_DEVICE_HEATING_COOLING_UNIT 0X0300
#define ZHA_DEVICE_THERMOSTAT 0X0301
#define ZHA_DEVICE_TEMPERATURE_SENSOR 0X0302
#define ZHA_DEVICE_PUMP 0X0303
#define ZHA_DEVICE_PUMP_CONTROLLER 0X0304
#define ZHA_DEVICE_PRESSURE_SENSOR 0X0305
#define ZHA_DEVICE_FLOW_SENSOR 0X0306

// Intruder alarm systems
#define ZHA_DEVICE_IAS_CONTROL_AND_INDICATING_EQUIPMENT 0X0400
#define ZHA_DEVICE_IAS_ANCILLARY_CONTROL_EQUIPMENT 0X0401
#define ZHA_DEVICE_IAS_ZONE 0X0402
#define ZHA_DEVICE_IAS_WARNING_DEVICE 0X0403

////////////////////////////////////////////////////////////////////////////////
/// ZHA requires Discover Endpoint. Provide some utilties to use easily
///

const wpan_cluster_table_entry_t digi_data_clusters[] = {
    XBEE_DISC_DIGI_DATA_CLUSTER_ENTRY, // Required for Home Automation Profile
    WPAN_CLUST_ENTRY_LIST_END
};

// endpoint, profile, handler, context, device, version, cluster list
#define DIGI_DISC_ENDPOINT()                                    \
   { WPAN_ENDPOINT_DIGI_DATA, WPAN_PROFILE_DIGI, NULL,         \
      NULL, 0x0000, 0x00, digi_data_clusters }
