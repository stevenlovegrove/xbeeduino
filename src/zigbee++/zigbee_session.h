#pragma once

#include <xbee/wpan.h>

#include "zigbee_endpoint.h"
#include "xbee_params.h"

template<typename... Endpoints>
class zigbee_session
{
public:
    constexpr static unsigned CUSTOM_ENDPOINTS = sizeof...(Endpoints);
    constexpr static unsigned TOTAL_ENDPOITNS = CUSTOM_ENDPOINTS+3;

    zigbee_session(Endpoints&... endpoints)
        : table{
              endpoints.entry()...,
              DIGI_DISC_ENDPOINT(),
              ZDO_ENDPOINT(ep_state_zdo),
              WPAN_ENDPOINT_TABLE_END
          }
    {
    }

    void start(const xbee_serial_t& xbee_serial_config, bool wait_for_modem_response)
    {
        check_xbee_result(xbee_dev_init( &xdev, &xbee_serial_config, NULL, NULL));
        check_xbee_result(xbee_wpan_init( &xdev, table));
        check_xbee_result(xbee_cmd_init_device( &xdev));

        if(wait_for_modem_response) {
            while(int status = xbee_cmd_query_status(&xdev)) {
                if(status == -EBUSY) {
                    process_pending_frames();
                }else{
                    // we're done or there has been an error.
                    check_xbee_result(status);
                    break;
                }
            }
        }
    }

    void process_pending_frames()
    {
        check_xbee_result(xbee_dev_tick(&xdev));
    }

    void send_at_command(const char command[3], uint8_t* data, uint8_t data_len, xbee_cmd_callback_fn callback)
    {
        const int16_t handle = xbee_cmd_create(&xdev, command);
        if(!check_xbee_result(handle)) return;

        if(data && data_len > 0) {
            check_xbee_result(xbee_cmd_set_param_bytes(handle, data, data_len));
        }

        if(callback) {
            check_xbee_result(xbee_cmd_set_callback(handle, callback, nullptr));
        }

        check_xbee_result(xbee_cmd_send(handle));
    }

    template<typename T>
    void set_param(xbee_param param, const T& value)
    {
        send_at_command(get_param_command(param), (uint8_t*)(&value), sizeof(T), &set_param_response);
    }

    void get_param(xbee_param param, xbee_cmd_callback_fn callback)
    {
        if(callback) {
            send_at_command(get_param_command(param), nullptr, 0, callback);
        }else{
            log("No callback specified.\n");
        }
    }

private:
    static int set_param_response(const xbee_cmd_response_t* response)
    {
        const char cmd[] = {response->command.str[0], response->command.str[1], '\0'};

        if(response->flags) {
            // Error
            log("Error (%d) setting param %s.\n", response->flags, cmd);
        }else{
            log("Success setting %s.\n", cmd);
        }
        return XBEE_ATCMD_DONE;
    }

    xbee_dev_t xdev;
    wpan_ep_state_t ep_state_zdo;
    const wpan_endpoint_table_entry_t table[TOTAL_ENDPOITNS+1]; // include LIST_END
};
