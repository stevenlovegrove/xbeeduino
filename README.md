# Zigbee Device for SmartThings (with Edge driver)

This repo contains:

1) An implementation of a [Zigbee](zigbeealliance.org) Home Automation 2-gang switch using the [XBee](www.digi.com/xbee) [c-library stack](https://github.com/digidotcom/xbee_ansic_library).
   The implementation is based heavily on [pxbee-trigger](https://github.com/exsilium/pxbee-trigger) (thanks [exsilium](https://github.com/exsilium)!) with **hc08a** application processor specifics removed and some simplification (based on the latest [Digi Xbee SDK](https://github.com/digidotcom/xbee_ansic_library) and a more C++-like coding style for those on less constrained devices).
   The repo includes a CMake script for building a PC host application for use with a USB XBee adapter such as [Adafruits](https://www.adafruit.com/product/247) or [SparkFuns](https://www.sparkfun.com/products/11812) but the eventual target for this example is Arduino (Work in progress).
2) A [SmartThings Edge](https://developer-preview.smartthings.com/docs/devices/hub-connected/get-started) driver for using the custom Zigbee device with the SmartThings hub and smartphone app.



## The Zigbee Device

### Radio module

Using the latest XBee c-library, I believe this repo should work with most XBee's but I've only tested with XBee Pro 2SB (XBP24BZ7WITB) and XBee Pro S2C (S2CTH). I don't have a lot of experience with the rest of their line. What I know I have learnt from places such as exsiliums [blog](https://sten.pw/programmable-xbee-zigbee-radio-development-in-linux-macos/) and my questions on his GitHub such as [this](https://github.com/exsilium/pxbee-fwup.js/issues/4). Note, the S2B has a programmable application processor (hc08a) and exsilium has some great info for working with this directly to remove the need for another microcontroller or host. This device can also be used via serial (the same as the only mode for the S2C), and this is how we will use it here.

### Connection to SmartThings (and probably other Home Automation hubs)

The radio modules must be configured to operate in a mode that is compatible with the Smart Hub.

**TODO**: Document following topics

* Introduce XCTU
  * Connecting (Serial params, Resets & Passthrough mode)
  * Choosing firmware
    * Mention that S2C has more memory and therefore firmware selection is not specific to role (unlike S2B)
* List params and link to sources of info
* First test - connecting to hub as a 'thing' without code
* Removing from SmartThings and Reconnecting
  * Recovering from a bad hub / device state

### Zigbee Device Code

Once your device is connected, it's convenient to run your device program on your PC for (arguably) much simpler debugging. For convenience, I've included a CMake build script which should work on Linux / OSX (A Windows PR would be a great contribution :D - ask for pointers if you're interested).

```
mkdir build && cd build
cmake ..
cmake --build .
./build/xbeeduino /dev/tty-usbserial-XXXX 115200
```



## The SmartThings Edge Driver

See [SmartThings Edge](https://developer-preview.smartthings.com/docs/devices/hub-connected/get-started) for getting started with the new Edge framework. This framework lets you support custom Zigbee (and other) devices with your SmartThings or Aeon Home Hub. You'll need hub firmware version 000.038.000XX or greater.

### Setup
Download the latest smarthings-cli [release](https://github.com/SmartThingsCommunity/smartthings-cli/releases) and extract to a location on your PATH

### Do Once

```
cd ./smartthingsedge
smartthings edge:drivers:package whichswitch/  # Compile whichswitch driver 
smartthings edge:channels:create               # Create a 'release channel' for all of your drivers
smartthings edge:channels:enroll               # Enrolle your driver into the channel
smartthings edge:drivers:publish               # Publish your driver to the channel
smartthings edge:drivers:install               # Install driver to the hub
```

### Install any modifications

```
cd ./smartthingsedge
[make changes to code...]
smartthings edge:drivers:package whichswitch/  # Compile whichswitch driver 
smartthings edge:drivers:publish               # Publish your driver to the channel
smartthings edge:drivers:install               # Install driver to the hub
```

### View Hub log output

TODO: Document how to view ouput

(I'm currently having issues https://community.smartthings.com/t/how-to-enable-logging-on-the-hub/230465/4)

