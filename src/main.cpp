// Project derived largely from https://github.com/exsilium/pxbee-trigger with some refactoring
// and simplification for my own education and code needs. Thanks exsilium!


#define ZCL_MANUFACTURER_NAME "lovegrovecorp"
#define ZCL_MODEL_IDENTIFIER  "whichswitch"
#define ZCL_POWER_SOURCE      ZCL_BASIC_PS_DC

#include <cstdint>

#include <wpan/types.h>
#include <wpan/aps.h>
#include <util.h>
#include <xbee/atcmd.h>
#include <xbee/sxa.h>
#include <zigbee/zdo.h>
#include <zigbee/zcl.h>
#include <zigbee/zcl_client.h>
#include <zigbee/zcl_basic.h>
#include <zigbee/zcl_basic_attributes.h>
#include <zigbee/zcl_onoff.h>

#include "util.h"
#include "home_automation.h"
#include "response_builder.h"
#include "zcl_level.h"
#include "cluster_handler.h"

////////////////////////////////////////////////////////////////////////////////
/// Application state / Logic
///

// Marker for whether we need to report an attribute value change.
// Mark as true to cause main loop to report current value
bool queue_update = false;

struct physical_switch
{
    physical_switch(uint8_t zigbee_endpoint, const char* friendly_name)
        : zigbee_endpoint(zigbee_endpoint), friendly_name(friendly_name), attribs{
            { ZCL_ONOFF_ATTR_ONOFF, ZCL_ATTRIB_FLAG_NONE, ZCL_TYPE_LOGICAL_BOOLEAN, &current_on_off},
            { ZCL_LEVEL_ATTR_CURRENT_LEVEL, ZCL_ATTRIB_FLAG_NONE, ZCL_TYPE_UNSIGNED_8BIT, &current_level},
            { ZCL_ATTRIBUTE_END_OF_LIST }
        }
    {
    }

    const uint8_t zigbee_endpoint;
    const char* friendly_name;
    bool_t current_on_off;
    uint8_t current_level;

    zcl_attribute_base_t attribs[3];
};

// Dummy state that we are managing (representing virtual switches)
constexpr unsigned num_switches = 2;
physical_switch switches[num_switches] = {
    {0x00, "Switch0"},
    {0x01, "Switch1"}
};

physical_switch& get_switch(uint8_t endpoint)
{
    for(physical_switch& s : switches) {
        if(endpoint == s.zigbee_endpoint)
            return s;
    }
    log("Invalid Endpoint Provided (%d). Returning first endpoint.", endpoint);
    return switches[0];
}

////////////////////////////////////////////////////////////////////////////////
/// Endpoint Handlers
///

void attrib_reply(zcl_command_t& zcl)
{
    physical_switch& s = get_switch(zcl.envelope->dest_endpoint);
    send_attrib_requests(zcl, s.attribs, zcl.length / 2, (uint16_t*)zcl.zcl_payload);
}

void config(zcl_command_t& zcl)
{
    physical_switch& s = get_switch(zcl.envelope->dest_endpoint);
    queue_update = true;
    send_configure_response(zcl, ZCL_STATUS_SUCCESS);
}

void on_off_cmd(zcl_command_t& zcl)
{
    physical_switch& s = get_switch(zcl.envelope->dest_endpoint);

    switch(zcl.command) {
    case ZCL_ONOFF_CMD_OFF:
        log("Turning %s off.\n", s.friendly_name);
        s.current_on_off = false;
        break;
    case ZCL_ONOFF_CMD_ON:
        log("Turning %s on.\n", s.friendly_name);
        s.current_on_off = true;
        break;
    case ZCL_ONOFF_CMD_TOGGLE:
        log("Toggling %s.\n", s.friendly_name);
        s.current_on_off = !s.current_on_off;
        break;
    default:
        log("Unrecognized command.\n");
        zcl_command_dump(&zcl);
        zcl_default_response(&zcl, ZCL_STATUS_UNSUP_GENERAL_COMMAND);
        return;
    }

    send_attrib_table_response(zcl, s.attribs);
}

void level_cmd(zcl_command_t& zcl)
{
    physical_switch& s = get_switch(zcl.envelope->dest_endpoint);

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
        s.current_level = level;
        if( s.current_on_off != level > 0 ) {
            s.current_on_off = level > 0;
        }
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

    send_attrib_table_response(zcl, s.attribs+1);
}

////////////////////////////////////////////////////////////////////////////////
/// Clusters
///

cluster_handler on_off_handler(ZCL_CLUST_ONOFF, config, attrib_reply, on_off_cmd);
cluster_handler level_handler(ZCL_CLUST_LEVEL_CONTROL, config, attrib_reply, level_cmd);

const wpan_cluster_table_entry_t switch_clusters[num_switches][4] = {
    { ZCL_CLUST_ENTRY_BASIC_SERVER, on_off_handler.entry(), level_handler.entry(), WPAN_CLUST_ENTRY_LIST_END },
    { ZCL_CLUST_ENTRY_BASIC_SERVER, on_off_handler.entry(), level_handler.entry(), WPAN_CLUST_ENTRY_LIST_END }
};

////////////////////////////////////////////////////////////////////////////////
/// Endpoints
///

// Global for the ZDO/ZCL state keeping
wpan_ep_state_t ep_state_zdo;
wpan_ep_state_t ep_state_switch[num_switches];

const wpan_endpoint_table_entry_t endpoints_table[] = {
    {switches[0].zigbee_endpoint, WPAN_PROFILE_HOME_AUTOMATION, zcl_invalid_cluster, &ep_state_switch[0], ZHA_DEVICE_ON_OFF_OUTPUT, 0x00, switch_clusters[0]},
    {switches[1].zigbee_endpoint, WPAN_PROFILE_HOME_AUTOMATION, zcl_invalid_cluster, &ep_state_switch[1], ZHA_DEVICE_ON_OFF_OUTPUT, 0x00, switch_clusters[1]},
    DIGI_DISC_ENDPOINT(),
    ZDO_ENDPOINT(ep_state_zdo),
    WPAN_ENDPOINT_TABLE_END
};

////////////////////////////////////////////////////////////////////////////////
/// Frame Handlers
///

// This is extern referenced from device.h and sets up main processing commands.
const xbee_dispatch_table_entry_t xbee_frame_handlers[] =
{
    XBEE_FRAME_HANDLE_RX_EXPLICIT,
    XBEE_FRAME_HANDLE_LOCAL_AT,
    XBEE_FRAME_MODEM_STATUS_DEBUG,
    XBEE_FRAME_TABLE_END
};

////////////////////////////////////////////////////////////////////////////////
/// Main loop
///

int main(int argc, char *argv[])
{
    if(argc != 3) {
        printf("Usage: %s ttydev baudrate\n", argv[0]);
        printf("  e.g. %s /dev/tty.usbserial-XXXXX 115200\n", argv[0]);
        return 0;
    }

    // Setup params
    xbee_serial_t xbee_serial_config = { 0, 0, "" };
    strncpy(xbee_serial_config.device, argv[1], sizeof(xbee_serial_config.device));
    xbee_serial_config.baudrate = atoi(argv[2]);

    xbee_dev_t xdev;
    xbee_node_id_t *target = NULL;

    // initialize the serial and device layer for this XBee device
    log("= Connecting to Radio device.\n");
    check_xbee_result(xbee_dev_init( &xdev, &xbee_serial_config, NULL, NULL));

    // Setup necessary XBee module settings
    log("= Setting zigbee home automation profile parameters.\n");
    setup_zigbee_homeautomation_params(xdev);

    // Enables endpoints and clusters
    log("= Initializing WPAN.\n");
    check_xbee_result(xbee_wpan_init( &xdev, endpoints_table));

    log("= Start frame processing loop\n");
    for (unsigned loop=0;; ++loop) {
        check_xbee_result(xbee_dev_tick(&xdev));
        if(queue_update) {
            for(unsigned i=0; i < num_switches; ++i) {
                report_attribs(xdev, switches[i].attribs, &(endpoints_table[i]));
            }
            queue_update = false;
        }
        delay(1);
    }
}


