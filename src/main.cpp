#include "zigbee++/zigbee_session.h"
#include "zigbee++/cluster_level.h"
#include "zigbee++/cluster_on_off.h"
#include "zigbee++/cluster_basic.h"

int main(int argc, char *argv[])
{
    if(argc != 3) {
        printf("Usage: %s ttydev baudrate\n", argv[0]);
        printf("  e.g. %s /dev/tty.usbserial-XXXXX 115200\n", argv[0]);
        return 0;
    }

    // We must setup some basic unique info which will alow us to find this device
    cluster_basic::set_manufacturer_name("lovegrovecorp");
    cluster_basic::set_model_identifier("whichswitch");

    // Endpoints are zigbee terminology for logical devices to talk with
    // We create two endpoints for two 'switches', each support 3 zigbee clusters
    // which are like standard capabilities.
    zigbee_endpoint<cluster_basic,cluster_on_off,cluster_level> endpoints[] = {
        {0x00, "switch0"}, {0x01, "switch1"}
    };

    // Setup default values and callbacks for our two endpoints
    for(auto& ep : endpoints) {
        ep.get<cluster_on_off>().current_value = false;
        ep.get<cluster_on_off>().fn_updated = [](cluster_on_off::event e) {
            log("Endpoint % turned %", e.endpoint, e.new_value ? "on" : "off");
            return true;
        };;

        ep.get<cluster_level>().current_level = 0x80;
        ep.get<cluster_level>().fn_updated = [](cluster_level::event e) {
            const int percent = (int)((100.0f/255.0f)*e.new_value+0.5f);
            log("Endpoint % set level to % (transition time %)", e.endpoint, percent, e.transition_time);
            return true;
        };
    }

    // Setup communication params for the xbee device
    xbee_serial_t xbee_serial_config = { 0, 0, "" };
    strncpy(xbee_serial_config.device, argv[1], sizeof(xbee_serial_config.device));
    xbee_serial_config.baudrate = atoi(argv[2]);

    // Initialize the zigbee stack with the endpoints and communication params
    zigbee_session session(endpoints[0], endpoints[1]);
    log("Connecting to Radio...\n");
    session.start(xbee_serial_config, true);
    log("Success!\n");

    // We control the processing loop (there are no threads)
    for (unsigned loop=0;; ++loop) {
        session.process_pending_frames();
        delay(1);
    }
}
