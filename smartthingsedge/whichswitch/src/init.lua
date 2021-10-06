-- article covers how to deal with multiple endpoints
-- https://developer-preview.smartthings.com/edge-device-drivers/zigbee/device.html

local capabilities = require "st.capabilities"
local ZigbeeDriver = require "st.zigbee"
local defaults = require "st.zigbee.defaults"
local log = require "log"
local clusters = require "st.zigbee.zcl.clusters"
local cluster_base = require "st.zigbee.cluster_base"

ENDPOINT1 = 0xA7
-- ENDPOINT2 = 0xA8

local function component_to_endpoint(device, component_id)
  if component_id == "main" then
    return ENDPOINT1
  -- elseif component_id == "relay2" then
  --   return ENDPOINT2
  end
end

local function endpoint_to_component(device, ep)
  if ep == ENDPOINT1 then
    return "main"
  -- elseif ep == ENDPOINT2 then
  --   return "relay2"
  end
end

local function level_report_handler(driver, device, value, zb_rx)
  local level = math.floor((value.value / 254.0 * 100) + 0.5)
  device:emit_event_for_endpoint(
    zb_rx.address_header.src_endpoint.value,
    capabilities.switchLevel.level(level)
  )
end

local function set_level_handler(driver, device, command)
  local dev_level = math.floor(command.args.level * 0xFE / 100)
  device:send_to_component(command.component, clusters.Level.commands.MoveToLevelWithOnOff(device, dev_level, 0xFFFF))
  -- device:emit_event(capabilities.switchLevel.level(command.args.level))
end

local device_init = function(self, device)
  device:set_component_to_endpoint_fn(component_to_endpoint)
  device:set_endpoint_to_component_fn(endpoint_to_component)
end

local zigbee_outlet_driver_template = {
  supported_capabilities = {
    capabilities.switch,
    capabilities.switchLevel,
    capabilities.refresh
  },
  zigbee_handlers = {
    attr = {
      [clusters.Level.ID] = {
        [clusters.Level.attributes.CurrentLevel.ID] = level_report_handler
      }
    }
  },
  capability_handlers = {
    [capabilities.switchLevel.ID] = {
      [capabilities.switchLevel.commands.setLevel.NAME] = set_level_handler
    }
  },
  lifecycle_handlers = {
    init = device_init,
  }
}

defaults.register_for_default_handlers(zigbee_outlet_driver_template, zigbee_outlet_driver_template.supported_capabilities)
local zigbee_outlet = ZigbeeDriver("whichswitch", zigbee_outlet_driver_template)
zigbee_outlet:run()
