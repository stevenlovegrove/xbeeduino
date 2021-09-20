#include <xbee/discovery.h>
#include <xbee/atcmd.h>
#include <zigbee/zcl.h>
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

////////////////////////////////////////////////////////////////////////////////
/// ZHA requires some particular XBee parameter values
///

// TODO: This doesn't work reliably for some reason. I keep having to set in XCTU
void setup_zigbee_homeautomation_params(xbee_dev_t& xdev)
{
    struct app_param {
        const char command[3];
        uint32_t value;
    };
    const app_param params[] = {
        {"ZS", 2}, {"AP", 1}, {"AO", 3}, {"EE", 1},
        {"NJ", 0x5A}, {"NH", 0x1E}, {"NO", 0}, {"EO", 1},
    };
    static const char security_key[] = "5A6967426565416C6C69616E63653039";

    for(auto& p : params) {
        log("Setting %s\n", p.command);
        check_xbee_result(xbee_cmd_simple(&xdev, p.command, p.value));
    }

    log("Setting KY Security Key\n");
    check_xbee_result(xbee_cmd_execute(&xdev, "KY", security_key, sizeof(security_key) - 1));
    log("WR\n");
    check_xbee_result(xbee_cmd_execute(&xdev, "WR", NULL, 0));
    log("Done!\n");
}

////////////////////////////////////////////////////////////////////////////////
/// Report ZHA attribute change
///

void report_attribs(xbee_dev_t& xdev, const zcl_attribute_base_t *attr_list, const wpan_endpoint_table_entry_t *source_endpoint) {
    // This is meant for device side triggered notifications on state change
    PACKED_STRUCT request {
        zcl_header_nomfg_t header;
        uint8_t payload[80];
    } request;
    int bytecount;
    int retval = 0;
    wpan_envelope_t envelope;

    wpan_envelope_create( &envelope, &xdev.wpan_dev, WPAN_IEEE_ADDR_COORDINATOR, WPAN_NET_ADDR_COORDINATOR);
    envelope.source_endpoint = source_endpoint->endpoint;
    envelope.dest_endpoint = 0x01;
    envelope.cluster_id = ZCL_CLUST_ONOFF;
    envelope.profile_id = WPAN_PROFILE_HOME_AUTOMATION;
    envelope.payload = &request;
    request.header.frame_control = ZCL_FRAME_SERVER_TO_CLIENT
            | ZCL_FRAME_TYPE_PROFILE
            | ZCL_FRAME_DISABLE_DEF_RESP;
    request.header.command = ZCL_CMD_REPORT_ATTRIB;

    bytecount = zcl_create_attribute_records(&request.payload, sizeof(request.payload), &attr_list);
    request.header.sequence = wpan_endpoint_next_trans( source_endpoint );
    envelope.length = offsetof(struct request, payload) + bytecount;
    wpan_envelope_send(&envelope);
}
