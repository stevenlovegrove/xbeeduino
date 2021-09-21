#pragma once

#include <xbee/wpan.h>

#include "zigbee_endpoint.h"

template<typename... Endpoints>
struct zigbee_session
{
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

    void start(const xbee_serial_t& xbee_serial_config)
    {
        check_xbee_result(xbee_dev_init( &xdev, &xbee_serial_config, NULL, NULL));
        check_xbee_result(xbee_wpan_init( &xdev, table));
    }

    void process_pending_frames()
    {
        check_xbee_result(xbee_dev_tick(&xdev));
    }

    xbee_dev_t xdev;
    wpan_ep_state_t ep_state_zdo;
    const wpan_endpoint_table_entry_t table[TOTAL_ENDPOITNS+1]; // include LIST_END
};
