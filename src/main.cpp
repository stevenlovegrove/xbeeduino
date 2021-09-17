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

////////////////////////////////////////////////////////////////////////////////
/// Application state / Logic
///

// Zigbee endpoint identifiers for our two controllable relays
constexpr uint8_t RELAY1_ENDPOINT = 0x00;
constexpr uint8_t RELAY2_ENDPOINT = 0x01;

bool queue_update = false;
bool_t relay1_value = false;
bool_t relay2_value = false;

bool get_relay(uint8_t endpoint) {
    if(endpoint == RELAY1_ENDPOINT) {
        return relay1_value;
    } else if(endpoint == RELAY2_ENDPOINT) {
        return relay2_value;
    } else {
        log("Invalid endpoint received for triggering relay, no action taken.\n");
        return false;
    }
}

void set_relay(uint8_t endpoint, bool on) {

    if(endpoint == RELAY1_ENDPOINT) {
        relay1_value = on;
        log("Set relay1 %s\n", on ? "on" : "off");
    } else if(endpoint == RELAY2_ENDPOINT) {
        relay2_value = on;
        log("Set relay2 %s\n", on ? "on" : "off");
    } else {
        log("Invalid endpoint received for triggering relay, no action taken.\n");
    }
}

////////////////////////////////////////////////////////////////////////////////
/// Endpoint Handlers
///

int custom_ep_rx_on_off_cluster(const wpan_envelope_t FAR *envelope, void FAR *context)
{
    zcl_command_t zcl;
    if(zcl_command_build(&zcl, envelope, nullptr) == 0) {
        log("=== ON OFF CLUSTER Handler (endpoint: %d) ===\n", zcl.envelope->dest_endpoint);

        response_builder_t<20> response_builder;

        // Handle all the commands
        if(ZCL_CMD_MATCH(&zcl.frame_control, GENERAL, CLIENT_TO_SERVER, PROFILE)) {
            if(zcl.command == ZCL_CMD_READ_ATTRIB) {
                log("Handling response for switch state read\n");
                // Could use zcl_encode_attribute_value() here.
                response_builder.send(zcl, ZCL_CMD_READ_ATTRIB_RESP) <<
                        0x00, 0x00, ZCL_STATUS_SUCCESS,
                        ZCL_TYPE_LOGICAL_BOOLEAN, get_relay(zcl.envelope->dest_endpoint);
            }
            else if(zcl.command == ZCL_CMD_CONFIGURE_REPORT) {
                log("Handling response for configure report command\n");
                response_builder.send(zcl, ZCL_CMD_CONFIGURE_REPORT_RESP) << ZCL_STATUS_SUCCESS, ZCL_DIRECTION_SEND, 0x00, 0x00;
                queue_update = true;
            } else if(zcl.command == ZCL_CMD_DEFAULT_RESP) {
                log("Got default response\n");
                zcl_command_dump(&zcl);
                return zcl_default_response(&zcl, ZCL_STATUS_SUCCESS);
            }else {
                log("Unhandled Profile command received\n");
                zcl_command_dump(&zcl);
                return zcl_default_response(&zcl, ZCL_STATUS_UNSUP_GENERAL_COMMAND);
            }
        }
        else if(ZCL_CMD_MATCH(&zcl.frame_control, GENERAL, CLIENT_TO_SERVER, CLUSTER)) {
            switch(zcl.command) {
            case ZCL_ONOFF_CMD_OFF:
                set_relay(zcl.envelope->dest_endpoint, false);
                break;

            case ZCL_ONOFF_CMD_ON:
                set_relay(zcl.envelope->dest_endpoint, true);
                break;

            case ZCL_ONOFF_CMD_TOGGLE:
                set_relay(zcl.envelope->dest_endpoint, !get_relay(zcl.envelope->dest_endpoint));
                break;

            default:
                log("Unhandled Cluster command received\n");
                zcl_command_dump(&zcl);
                return zcl_default_response(&zcl, ZCL_STATUS_UNSUP_CLUSTER_COMMAND);
            }

            // Respond with new state
            response_builder.send(zcl, ZCL_CMD_REPORT_ATTRIB, true) << 0x00, 0x00, ZCL_TYPE_LOGICAL_BOOLEAN, get_relay(zcl.envelope->dest_endpoint);
        }
    } else {
        log("Error!\n");
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////////
/// Attributes
/// These ZCL Attribute metadata structs for our boolean state makes report generation super easy.

const zcl_attribute_base_t relay1_attributes[] = {
    { ZCL_ONOFF_ATTR_ONOFF, ZCL_ATTRIB_FLAG_NONE, ZCL_TYPE_LOGICAL_BOOLEAN, &relay1_value},
    { ZCL_ATTRIBUTE_END_OF_LIST }
};
const zcl_attribute_tree_t relay1_tree[] =   {
    { ZCL_MFG_NONE, &relay1_attributes[0], NULL }
};

const zcl_attribute_base_t relay2_attributes[] = {
    { ZCL_ONOFF_ATTR_ONOFF, ZCL_ATTRIB_FLAG_NONE, ZCL_TYPE_LOGICAL_BOOLEAN, &relay1_value},
    { ZCL_ATTRIBUTE_END_OF_LIST }
};
const zcl_attribute_tree_t relay2_tree[] =   {
    { ZCL_MFG_NONE, &relay1_attributes[0], NULL }
};

////////////////////////////////////////////////////////////////////////////////
/// Clusters
///

const wpan_cluster_table_entry_t relay1_clusters[] = {
    ZCL_CLUST_ENTRY_BASIC_SERVER,
    {ZCL_CLUST_ONOFF, custom_ep_rx_on_off_cluster, zcl_attributes_none, WPAN_CLUST_FLAG_INPUT},
    WPAN_CLUST_ENTRY_LIST_END
};

const wpan_cluster_table_entry_t relay2_clusters[] = {
    ZCL_CLUST_ENTRY_BASIC_SERVER,
    {ZCL_CLUST_ONOFF, custom_ep_rx_on_off_cluster, zcl_attributes_none, WPAN_CLUST_FLAG_INPUT},
    WPAN_CLUST_ENTRY_LIST_END
};

////////////////////////////////////////////////////////////////////////////////
/// Endpoints
///

// Global for the ZDO/ZCL state keeping
wpan_ep_state_t ep_state_zdo;
wpan_ep_state_t ep_state_relay1;
wpan_ep_state_t ep_state_relay2;

const wpan_endpoint_table_entry_t endpoints_table[] = {
    {RELAY1_ENDPOINT, WPAN_PROFILE_HOME_AUTOMATION, zcl_invalid_cluster, &ep_state_relay1, ZHA_DEVICE_ON_OFF_OUTPUT, 0x00, relay1_clusters},
    {RELAY2_ENDPOINT, WPAN_PROFILE_HOME_AUTOMATION, zcl_invalid_cluster, &ep_state_relay2, ZHA_DEVICE_ON_OFF_OUTPUT, 0x00, relay2_clusters},
    ZDO_ENDPOINT(ep_state_zdo),
    DIGI_DISC_ENDPOINT(),
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
            report_attrib(xdev, relay1_attributes, &endpoints_table[0]);
            report_attrib(xdev, relay2_attributes, &endpoints_table[1]);
            queue_update = false;
        }
        delay(1);
    }
}


