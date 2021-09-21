#pragma once

#include <tuple>

#include <zigbee/zcl.h>
#include <zigbee/zcl_basic.h>
#include <zigbee/zcl_basic_attributes.h>
#include <zigbee/zdo.h>

#include "cluster_interface.h"
#include "cluster_attrib_utils.h"
#include "log.h"

template<typename... Clusters>
struct zigbee_endpoint
{
    constexpr static unsigned NUM_CLUSTERS = sizeof...(Clusters);

    zigbee_endpoint(uint8_t zigbee_endpoint_id, const char* friendly_name)
        : zigbee_endpoint_id(zigbee_endpoint_id), friendly_name(friendly_name),
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
            zigbee_endpoint_id, WPAN_PROFILE_HOME_AUTOMATION,
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
    // Base case for variadic recursion below
    template<typename C>
    cluster_interface* get_cluster_by_id(uint16_t cluster_id)
    {
        if( C::CLUSTOR_ID == cluster_id ) {
            return dynamic_cast<cluster_interface*>(&std::get<C>(clusters));
        }
        return nullptr;
    }

    // Glorified switch statement to map from cluster_id into clusters tuple.
    template<typename C, typename Cn, typename... Cs>
    cluster_interface* get_cluster_by_id(uint16_t cluster_id)
    {
        cluster_interface* c = get_cluster_by_id<C>(cluster_id);
        return c ? c : get_cluster_by_id<Cn, Cs...>(cluster_id);
    }

    // Handle any messages for clusters in this endpoint
    int dispatch_handler(const wpan_envelope_t *envelope)
    {
        zcl_command_t zcl;
        if(zcl_command_build(&zcl, envelope, nullptr) == 0) {
            cluster_interface* cluster = get_cluster_by_id<Clusters...>(envelope->cluster_id);
            assert(cluster);

            if(ZCL_CMD_MATCH(&zcl.frame_control, GENERAL, CLIENT_TO_SERVER, PROFILE)) {
                if(zcl.command == ZCL_CMD_READ_ATTRIB) {
                    assert(zcl.length%2 ==0);
                    send_attrib_requests(zcl, cluster->get_attribs(), zcl.length / 2, (uint16_t*)zcl.zcl_payload);
                    return 0;
                } else if(zcl.command == ZCL_CMD_CONFIGURE_REPORT) {
                    cluster->config(zcl);
                    return 0;
                } else if(zcl.command == ZCL_CMD_DEFAULT_RESP) {
                    return zcl_default_response(&zcl, ZCL_STATUS_SUCCESS);
                }else {
                    log("Unhandled Profile command received\n");
                    zcl_command_dump(&zcl);
                    return zcl_default_response(&zcl, ZCL_STATUS_UNSUP_GENERAL_COMMAND);
                }
            }else if(ZCL_CMD_MATCH(&zcl.frame_control, GENERAL, CLIENT_TO_SERVER, CLUSTER)) {
                cluster->command(zcl);
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
        zigbee_endpoint* self = static_cast<zigbee_endpoint*>(context);
        return self->dispatch_handler(envelope);
    }

    const uint8_t zigbee_endpoint_id;
    const char* friendly_name;
    wpan_ep_state_t state;
    std::tuple<Clusters...> clusters;
    zcl_attribute_base_t* attrib_table_for_cluster[NUM_CLUSTERS];
    wpan_cluster_table_entry_t clustertable[NUM_CLUSTERS+2]; // including BASIC + LIST_END
};
