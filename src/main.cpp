// These defines are used within <zigbee/zcl_basic_attributes.h>
// for reporting to communicating devices.
#define ZCL_MANUFACTURER_NAME "lovegrovecorp"
#define ZCL_MODEL_IDENTIFIER  "whichswitch"
#define ZCL_POWER_SOURCE      ZCL_BASIC_PS_DC

#include "zigbee++/zigbee_session.h"
#include "zigbee++/cluster_level.h"
#include "zigbee++/cluster_on_off.h"
#include "zigbee++/util.h"

zigbee_endpoint<cluster_on_off,cluster_level> ep0(0x00, "ep0");
zigbee_endpoint<cluster_on_off,cluster_level> ep1(0x01, "ep1");
zigbee_session session(ep0, ep1);

int main(int argc, char *argv[])
{
    if(argc != 3) {
        printf("Usage: %s ttydev baudrate\n", argv[0]);
        printf("  e.g. %s /dev/tty.usbserial-XXXXX 115200\n", argv[0]);
        return 0;
    }

    ep0.get<cluster_on_off>().user_fn_try_update = [](){
        log("ep0 switch\n");
    };
    ep1.get<cluster_on_off>().user_fn_try_update = [](){
        log("ep1 switch\n");
    };

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
