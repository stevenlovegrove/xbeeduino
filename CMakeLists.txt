cmake_minimum_required(VERSION 3.20)
project(xbeeduino)
set (CMAKE_CXX_STANDARD 11)

include_directories(${CMAKE_CURRENT_LIST_DIR}/xbee/include)

set(XBEE_SRC
    "xbee/ports/posix/xbee_platform_posix.c"
#    "xbee/ports/posix/xbee_readline.c"
    "xbee/ports/posix/xbee_serial_posix.c"
    "xbee/src/wpan/wpan_aps.c"
    "xbee/src/wpan/wpan_types.c"
    "xbee/src/xbee/xbee_atcmd.c"
    "xbee/src/xbee/xbee_device.c"
    "xbee/src/xbee/xbee_wpan.c"
    "xbee/src/xbee/xbee_time.c"
    "xbee/src/xbee/xbee_discovery.c"
    "xbee/src/zigbee/zcl_basic.c"
    "xbee/src/zigbee/zcl_client.c"
    "xbee/src/zigbee/zcl_time.c"
    "xbee/src/zigbee/zigbee_zcl.c"
    "xbee/src/zigbee/zigbee_zdo.c"
    "xbee/src/zigbee/zcl_types.c"
    "xbee/src/util/hexdump.c"
    "xbee/src/util/swapbytes.c"
    "xbee/src/util/swapcpy.c"
    "xbee/src/util/hexstrtobyte.c"
)

add_executable(xbeeduino src/main.cpp ${XBEE_SRC} )
target_compile_definitions(xbeeduino PUBLIC POSIX XBEE_STD_CONFIG=0)
#target_compile_definitions(xbeeduino PUBLIC XBEE_DEVICE_VERBOSE XBEE_ATCMD_VERBOSE XBEE_WPAN_VERBOSE ZCL_BASIC_VERBOSE ZIGBEE_ZCL_VERBOSE ZIGBEE_ZDO_VERBOSE)
