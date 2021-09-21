#pragma once

#include <tuple>
#include <zigbee/zcl.h>
#include <zigbee/zcl_basic.h>
#include <zigbee/zcl_basic_attributes.h>
#include <zigbee/zdo.h>
#include "cluster_handler.h"
#include <zigbee/zcl_onoff.h>
#include "zcl_level.h"
#include "home_automation.h"

struct clustor_on_off
{
    constexpr static uint16_t CLUSTOR_ID = ZCL_CLUST_ONOFF;
    using fn_try_update = void(void);

    clustor_on_off()
        : user_fn_try_update(nullptr),
          attributes{
            { ZCL_ONOFF_ATTR_ONOFF, ZCL_ATTRIB_FLAG_NONE, ZCL_TYPE_LOGICAL_BOOLEAN, &current_value},
            { ZCL_ATTRIBUTE_END_OF_LIST }
        } {}

    void config(zcl_command_t& zcl)
    {
        send_configure_response(zcl, ZCL_STATUS_SUCCESS);
    }

    void command(zcl_command_t& zcl)
    {
        switch(zcl.command) {
        case ZCL_ONOFF_CMD_OFF:
            log("Turning off.\n");
            current_value = false;
            break;
        case ZCL_ONOFF_CMD_ON:
            log("Turning on.\n");
            current_value = true;
            break;
        case ZCL_ONOFF_CMD_TOGGLE:
            log("Toggling.\n");
            current_value = !current_value;
            break;
        default:
            log("Unrecognized command.\n");
            zcl_command_dump(&zcl);
            zcl_default_response(&zcl, ZCL_STATUS_UNSUP_GENERAL_COMMAND);
            return;
        }
        if(user_fn_try_update) user_fn_try_update();
        send_attrib_table_response(zcl, attributes);
    }


    fn_try_update* user_fn_try_update;
    zcl_attribute_base_t attributes[2];
    bool_t current_value;
};

struct clustor_level
{
    constexpr static uint16_t CLUSTOR_ID = ZCL_CLUST_LEVEL_CONTROL;
    using fn_try_update = void(void);

    clustor_level()
        : user_fn_try_update(nullptr),
          attributes{
            { ZCL_LEVEL_ATTR_CURRENT_LEVEL, ZCL_ATTRIB_FLAG_NONE, ZCL_TYPE_UNSIGNED_8BIT, &current_level},
            { ZCL_ATTRIBUTE_END_OF_LIST }
        } {}

    void config(zcl_command_t& zcl)
    {
        // TODO: Actually configure from params sent
        send_configure_response(zcl, ZCL_STATUS_SUCCESS);
    }

    void command(zcl_command_t& zcl)
    {
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
            current_level = level;
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
        if(user_fn_try_update) user_fn_try_update();
        send_attrib_table_response(zcl, attributes);
    }

    fn_try_update* user_fn_try_update;
    zcl_attribute_base_t attributes[2];
    uint8_t current_level;
};

template<typename... Clusters>
struct custom_zha_endpoint
{
    constexpr static unsigned NUM_CLUSTERS = sizeof...(Clusters);

    custom_zha_endpoint(uint8_t zigbee_endpoint, const char* friendly_name)
        : zigbee_endpoint(zigbee_endpoint), friendly_name(friendly_name),
          attrib_table_for_cluster{
              std::get<Clusters>(clusters).attributes...
          },
          clustertable{
              ZCL_CLUST_ENTRY_BASIC_SERVER,
              {Clusters::CLUSTOR_ID, &dispatch_handler, this, WPAN_CLUST_FLAG_INPUT}...,
              WPAN_CLUST_ENTRY_LIST_END
            }
    {
    }

    wpan_endpoint_table_entry_t entry()
    {
        return {
            zigbee_endpoint, WPAN_PROFILE_HOME_AUTOMATION,
            zcl_invalid_cluster, &state, ZHA_DEVICE_ON_OFF_OUTPUT,
            0x00, clustertable
        };
    }

    template<typename C>
    C& get()
    {
        return std::get<C>(clusters);
    }

private:
    zcl_attribute_base_t* get_attribs_for_cluster(uint16_t cluster_id) {
        for(unsigned i=0; i < NUM_CLUSTERS; ++i) {
            if(clustertable[i].cluster_id == cluster_id) {
                return attrib_table_for_cluster[i];
            }
        }
        return nullptr;
    }

    template<typename C>
    void forward_cluster_command(zcl_command_t& zcl) {
        if( C::CLUSTOR_ID == zcl.envelope->cluster_id ) {
            std::get<C>(clusters).command(zcl);
        }
    }

    template<typename C, typename Cn, typename... Cs>
    void forward_cluster_command(zcl_command_t& zcl)
    {
        forward_cluster_command<C>(zcl);
        forward_cluster_command<Cn, Cs...>(zcl);
    }


    // specific to this endpoint, any clustor
    int dispatch_handler(const wpan_envelope_t *envelope)
    {
        zcl_command_t zcl;
        if(zcl_command_build(&zcl, envelope, nullptr) == 0) {
            // Handle all the commands
            if(ZCL_CMD_MATCH(&zcl.frame_control, GENERAL, CLIENT_TO_SERVER, PROFILE)) {
                if(zcl.command == ZCL_CMD_READ_ATTRIB) {
                    assert(zcl.length%2 ==0);
                    zcl_attribute_base_t* attribs = get_attribs_for_cluster(zcl.envelope->cluster_id);
                    if(attribs) {
                        send_attrib_requests(zcl, attribs, zcl.length / 2, (uint16_t*)zcl.zcl_payload);
                    }else{
                        return zcl_default_response(&zcl, ZCL_STATUS_INVALID_SELECTOR);
                    }
                    return 0;
                } else if(zcl.command == ZCL_CMD_CONFIGURE_REPORT) {
                    log("Handling response for configure report command\n");
                    // TODO: Clustor specific configuration
                    send_configure_response(zcl, ZCL_STATUS_SUCCESS);
                    return 0;
                } else if(zcl.command == ZCL_CMD_DEFAULT_RESP) {
                    log("Got default response\n");
                    return zcl_default_response(&zcl, ZCL_STATUS_SUCCESS);
                }else {
                    log("Unhandled Profile command received\n");
                    zcl_command_dump(&zcl);
                    return zcl_default_response(&zcl, ZCL_STATUS_UNSUP_GENERAL_COMMAND);
                }
            }else if(ZCL_CMD_MATCH(&zcl.frame_control, GENERAL, CLIENT_TO_SERVER, CLUSTER)) {
                forward_cluster_command<Clusters...>(zcl);
                return 0;
            }else{
                log("Unrecognized cluster message.\n");
                zcl_command_dump(&zcl);
                return zcl_default_response(&zcl, ZCL_STATUS_UNSUP_GENERAL_COMMAND);
            }
        } else {
            log("Unable to build ZCL Structure.\n");
            return 0;
        }
        return 0;
    }

    static int dispatch_handler(const wpan_envelope_t *envelope, void* context)
    {
        custom_zha_endpoint* self = static_cast<custom_zha_endpoint*>(context);
        return self->dispatch_handler(envelope);
    }

    const uint8_t zigbee_endpoint;
    const char* friendly_name;
    wpan_ep_state_t state;
    std::tuple<Clusters...> clusters;
    zcl_attribute_base_t* attrib_table_for_cluster[NUM_CLUSTERS];
    wpan_cluster_table_entry_t clustertable[NUM_CLUSTERS+2]; // including BASIC + LIST_END
};

template<typename... Endpoints>
struct custom_endpoint_table
{
    constexpr static unsigned CUSTOM_ENDPOINTS = sizeof...(Endpoints);
    constexpr static unsigned TOTAL_ENDPOITNS = CUSTOM_ENDPOINTS+3;

    custom_endpoint_table(Endpoints... endpoints)
        : table{
              endpoints.entry()...,
              DIGI_DISC_ENDPOINT(),
              ZDO_ENDPOINT(ep_state_zdo),
              WPAN_ENDPOINT_TABLE_END
          }
    {

    }

    wpan_ep_state_t ep_state_zdo;
    const wpan_endpoint_table_entry_t table[TOTAL_ENDPOITNS+1]; // include LIST_END
};

