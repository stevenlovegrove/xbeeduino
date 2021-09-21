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

    cluster_basic::set_manufacturer_name("lovegrovecorp");
    cluster_basic::set_model_identifier("whichswitch");

    zigbee_endpoint<cluster_basic,cluster_on_off,cluster_level> endpoints[] = {
        {0x00, "switch0"}, {0x01, "switch1"}
    };

    zigbee_session session(endpoints[0], endpoints[1]);

    auto updated_on_off = [](cluster_on_off::event e) {
        log("Endpoint %d turned %s\n", e.endpoint, e.new_value ? "on" : "off");
        return true;
    };
    auto updated_level = [](cluster_level::event e) {
        const int percent = (int)((100.0f/255.0f)*e.new_value+0.5f);
        log("Endpoint %d set level to %d%% (transition time %d)\n", e.endpoint, percent, e.transition_time);
        return true;
    };

    endpoints[0].get<cluster_on_off>().fn_updated = updated_on_off;
    endpoints[1].get<cluster_on_off>().fn_updated = updated_on_off;
    endpoints[0].get<cluster_level>().fn_updated = updated_level;
    endpoints[1].get<cluster_level>().fn_updated = updated_level;

    // Setup params
    xbee_serial_t xbee_serial_config = { 0, 0, "" };
    strncpy(xbee_serial_config.device, argv[1], sizeof(xbee_serial_config.device));
    xbee_serial_config.baudrate = atoi(argv[2]);

    session.start(xbee_serial_config);

    for (unsigned loop=0;; ++loop) {
        session.process_pending_frames();
        delay(1);
    }
}
