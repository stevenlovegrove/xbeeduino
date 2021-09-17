-- article covers how to deal with multiple endpoints
-- https://developer-preview.smartthings.com/edge-device-drivers/zigbee/device.html

local capabilities = require "st.capabilities"
local ZigbeeDriver = require "st.zigbee"
local defaults = require "st.zigbee.defaults"
local log = require "log"
local zcl_clusters = require "st.zigbee.zcl.clusters"

local function component_to_endpoint(device, component_id)
  if component_id == "main" then
    return 0x00
  elseif component_id == "relay2" then
    return 0x01
  end
end

local function endpoint_to_component(device, ep)
  if ep == 0x00 then
    return "main"
  elseif ep == 0x01 then
    return "relay2"
  end
end

local device_init = function(self, device)
  device:set_component_to_endpoint_fn(component_to_endpoint)
  device:set_endpoint_to_component_fn(endpoint_to_component)
end

local zigbee_outlet_driver_template = {
  supported_capabilities = {
    capabilities.switch,
    capabilities.refresh
  },
  lifecycle_handlers = {
    init = device_init,
  },
}

defaults.register_for_default_handlers(zigbee_outlet_driver_template, zigbee_outlet_driver_template.supported_capabilities)
local zigbee_outlet = ZigbeeDriver("whichswitch", zigbee_outlet_driver_template)
zigbee_outlet:run()