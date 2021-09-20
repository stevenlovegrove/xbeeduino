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

////////////////////////////////////////////////////////////////////////////////
/// Application state / Logic
///

struct physical_switch
{
    const uint8_t zigbee_endpoint;
    const char* friendly_name;
    bool_t current_on_off;
    uint8_t current_level;
};

// Dummy state that we are managing (representing virtual switches)
constexpr unsigned num_switches = 2;
physical_switch switches[num_switches] = {
    {0x00, "Switch0", false, 255u},
    {0x01, "Switch1", false, 128u}
};

physical_switch* get_switch(uint8_t endpoint)
{
    for(physical_switch& s : switches) {
        if(endpoint == s.zigbee_endpoint)
            return &s;
    }
    return nullptr;
}

// Marker for whether we need to report an attribute value change.
// Mark as true to cause main loop to report current value
bool queue_update = false;

////////////////////////////////////////////////////////////////////////////////
/// Endpoint Handlers
///

/// Handle PROFILE and CLUSTER Zigbee messages for ZCL_CLUST_ONOFF cluster
/// This is the main application radio logic. \p envelope contains the received packet data.
/// The packet is directed to the endpoint `zcl.envelope->dest_endpoint` which for us
/// will be RELAY1_ENDPOINT or RELAY2_ENDPOINT
int rx_handler_on_off_cluster(const wpan_envelope_t FAR *envelope, void FAR */*context*/)
{
    zcl_command_t zcl;
    if(zcl_command_build(&zcl, envelope, nullptr) == 0) {
        log("=== ON OFF CLUSTER Handler (endpoint: %d) ===\n", zcl.envelope->dest_endpoint);
        physical_switch* s = get_switch(zcl.envelope->dest_endpoint);
        if(!s) {
            log("Message received for bad endpoint!\n");
            zcl_command_dump(&zcl);
            return zcl_default_response(&zcl, ZCL_STATUS_INVALID_VALUE);
        }

        response_builder_t<40> response_builder;

        // Handle all the commands
        if(ZCL_CMD_MATCH(&zcl.frame_control, GENERAL, CLIENT_TO_SERVER, PROFILE)) {
            if(zcl.command == ZCL_CMD_READ_ATTRIB) {
                log("Handling response for switch state read\n");
                // Could use zcl_encode_attribute_value() here.
                response_builder.send(zcl, ZCL_CMD_READ_ATTRIB_RESP) <<
                            0x00, 0x00, ZCL_STATUS_SUCCESS,
                            ZCL_TYPE_LOGICAL_BOOLEAN, s->current_on_off;
            } else if(zcl.command == ZCL_CMD_CONFIGURE_REPORT) {
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
        } else if(ZCL_CMD_MATCH(&zcl.frame_control, GENERAL, CLIENT_TO_SERVER, CLUSTER)) {
            switch(zcl.command) {
            case ZCL_ONOFF_CMD_OFF:
                log("Turning %s off.\n", s->friendly_name);
                s->current_on_off = false;
                break;

            case ZCL_ONOFF_CMD_ON:
                log("Turning %s on.\n", s->friendly_name);
                s->current_on_off = true;
                break;

            case ZCL_ONOFF_CMD_TOGGLE:
                log("Toggling %s.\n", s->friendly_name);
                s->current_on_off = !s->current_on_off;
                break;

            default:
                log("Unhandled Cluster command received\n");
                zcl_command_dump(&zcl);
                return zcl_default_response(&zcl, ZCL_STATUS_UNSUP_CLUSTER_COMMAND);
            }

            // Respond with new state
            response_builder.send(zcl, ZCL_CMD_REPORT_ATTRIB, true) << 0x00, 0x00, ZCL_TYPE_LOGICAL_BOOLEAN, s->current_on_off;
        }
    } else {
        log("Error!\n");
    }

    return 0;
}

int rx_handler_level_control_cluster(const wpan_envelope_t FAR *envelope, void FAR */*context*/)
{
    zcl_command_t zcl;
    if(zcl_command_build(&zcl, envelope, nullptr) == 0) {
        log("=== LEVEL CLUSTER Handler (endpoint: %d) ===\n", zcl.envelope->dest_endpoint);
        physical_switch* s = get_switch(zcl.envelope->dest_endpoint);
        if(!s) {
            log("Message received for bad endpoint!\n");
            zcl_command_dump(&zcl);
            return zcl_default_response(&zcl, ZCL_STATUS_INVALID_VALUE);
        }

        response_builder_t<40> response_builder;

        // Handle all the commands
        if(ZCL_CMD_MATCH(&zcl.frame_control, GENERAL, CLIENT_TO_SERVER, PROFILE)) {
            if(zcl.command == ZCL_CMD_READ_ATTRIB) {
                response_builder.send(zcl, ZCL_CMD_READ_ATTRIB_RESP) <<
                        0x00, 0x00, ZCL_STATUS_SUCCESS,
                        ZCL_LEVEL_ATTR_CURRENT_LEVEL_TYPE, s->current_level;
            } else if(zcl.command == ZCL_CMD_CONFIGURE_REPORT) {
                // TODO: This actually will contain information we're meant to use
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
        } else if(ZCL_CMD_MATCH(&zcl.frame_control, GENERAL, CLIENT_TO_SERVER, CLUSTER)) {
            switch(zcl.command) {
            case ZCL_LEVEL_CMD_MOVE: log("ZCL_LEVEL_CMD_MOVE\n"); break;
            case ZCL_LEVEL_CMD_STEP: log("ZCL_LEVEL_CMD_STEP\n"); break;
            case ZCL_LEVEL_CMD_STOP: log("ZCL_LEVEL_CMD_STOP\n"); break;

            case ZCL_LEVEL_CMD_MOVE_TO_LEVEL:
                [[fallthrough]];
            case ZCL_LEVEL_CMD_MOVE_TO_LEVEL_ON_OFF:
            {
                const uint8_t level = get_from_le_buffer<uint8_t>(zcl.zcl_payload,0);
                const uint16_t transition_time = get_from_le_buffer<uint8_t>(zcl.zcl_payload,1);
                log("Setting level to %d%% (transition time %d\n", (int)((100.0f/255.0f)*level+0.5f), transition_time);
                s->current_level = level;
                if( s->current_on_off != level > 0 ) {
                    s->current_on_off = level > 0;
                    queue_update = true;
                }
                break;
            }
            case ZCL_LEVEL_CMD_MOVE_ON_OFF: log("ZCL_LEVEL_CMD_MOVE_ON_OFF\n"); break;
            case ZCL_LEVEL_CMD_STEP_ON_OFF: log("ZCL_LEVEL_CMD_STEP_ON_OFF\n"); break;
            case ZCL_LEVEL_CMD_STOP_ON_OFF: log("ZCL_LEVEL_CMD_STOP_ON_OFF\n"); break;
            default:
                log("Unhandled Cluster command received\n");
                zcl_command_dump(&zcl);
                return zcl_default_response(&zcl, ZCL_STATUS_UNSUP_CLUSTER_COMMAND);
            }

            // Respond with new state
            response_builder.send(zcl, ZCL_CMD_REPORT_ATTRIB, true) << 0x00, 0x00, ZCL_LEVEL_ATTR_CURRENT_LEVEL_TYPE, s->current_level;
        }
    } else {
        log("Error!\n");
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////////
/// Attributes
/// These ZCL Attribute metadata structs for our boolean state makes report generation super easy.

const zcl_attribute_base_t switch_attributes[num_switches][3] = {
    {
        { ZCL_ONOFF_ATTR_ONOFF, ZCL_ATTRIB_FLAG_NONE, ZCL_TYPE_LOGICAL_BOOLEAN, &switches[0].current_on_off},
        { ZCL_LEVEL_ATTR_CURRENT_LEVEL, ZCL_ATTRIB_FLAG_NONE, ZCL_LEVEL_ATTR_CURRENT_LEVEL_TYPE, &switches[0].current_level},
        { ZCL_ATTRIBUTE_END_OF_LIST }
    },
    {
        { ZCL_ONOFF_ATTR_ONOFF, ZCL_ATTRIB_FLAG_NONE, ZCL_TYPE_LOGICAL_BOOLEAN, &switches[1].current_on_off},
        { ZCL_LEVEL_ATTR_CURRENT_LEVEL, ZCL_ATTRIB_FLAG_NONE, ZCL_LEVEL_ATTR_CURRENT_LEVEL_TYPE, &switches[1].current_level},
        { ZCL_ATTRIBUTE_END_OF_LIST }
    }
};

////////////////////////////////////////////////////////////////////////////////
/// Clusters
///

const wpan_cluster_table_entry_t switch_clusters[num_switches][4] = {
    {
        ZCL_CLUST_ENTRY_BASIC_SERVER,
        {ZCL_CLUST_ONOFF, rx_handler_on_off_cluster, zcl_attributes_none, WPAN_CLUST_FLAG_INPUT},
        {ZCL_CLUST_LEVEL_CONTROL, rx_handler_level_control_cluster, zcl_attributes_none, WPAN_CLUST_FLAG_INPUT},
        WPAN_CLUST_ENTRY_LIST_END
    },
    {
        ZCL_CLUST_ENTRY_BASIC_SERVER,
        {ZCL_CLUST_ONOFF, rx_handler_on_off_cluster, zcl_attributes_none, WPAN_CLUST_FLAG_INPUT},
        {ZCL_CLUST_LEVEL_CONTROL, rx_handler_level_control_cluster, zcl_attributes_none, WPAN_CLUST_FLAG_INPUT},
        WPAN_CLUST_ENTRY_LIST_END
    }
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
                report_attribs(xdev, switch_attributes[i], &(endpoints_table[i]));
            }
            queue_update = false;
        }
        delay(1);
    }
}


