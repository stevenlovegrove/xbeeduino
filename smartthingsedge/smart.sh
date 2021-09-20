#!/bin/bash

driver="whichswitch"
channel="personal-drivers"

# Optionally fill these in (but they might change and break)
# Leave them as is and they'll be looked up (but take a few seconds)
driver_id=""
channel_id=""
hub_id=""

if [ $# -eq 0 ]; then
    echo "No arguments provided"
    echo "Choose [setup,install,reset]"
    exit 1
fi

get_first_hub_id() {
    local YAML=$(smartthings devices --yaml)
    local regex=': ([a-z0-9\-]+)[^:]+:[^:]+: Home Hub'
    [[ $YAML =~ $regex ]]
    hub_id=${BASH_REMATCH[1]}
}

get_driver_id() {
  local driver_name=$1
  local YAML=$(smartthings edge:drivers --yaml)
  local regex=': ([a-z0-9\-]+)[^:]+:[^:]+: '$driver_name
  [[ $YAML =~ $regex ]]
  driver_id=${BASH_REMATCH[1]}
}

get_channel_id() {
  local channel_name=$1
  local YAML=$(smartthings edge:channels --yaml)
  local regex=': ([a-z0-9\-]+)[^:]+: '$channel_name
  [[ $YAML =~ $regex ]]
  channel_id=${BASH_REMATCH[1]}
}

setup() {
    # if not specified, get first available hub
    [ -z "$hub_id" ] && get_first_hub_id

    echo "Create a release channel name. Use '$channel' to continue using this script."
    smartthings edge:channels:create

    echo "Enrolling hub '$hub_id' into channel '$channel'"
    get_channel_id
    smartthings edge:channels:enroll -H $hub_id -C $channel_id
}

install() {
    echo "Packaging driver '$driver'"
    smartthings edge:drivers:package $driver

    [ -z "$driver_id" ] && get_driver_id $driver
    [ -z "$channel_id" ] && get_channel_id $channel
    echo "Publishing driver to channel '$channel'"
    smartthings edge:drivers:publish -C $channel_id $driver_id

    [ -z "$hub_id" ] && get_first_hub_id
    echo "Installing to hub '$hub_id'"
    smartthings edge:drivers:install -H $hub_id -C $channel_id $driver_id
}

reset() {
    # if not specified, get first available hub
    [ -z "$hub_id" ] && get_first_hub_id
    [ -z "$channel_id" ] && get_channel_id $channel
    [ -z "$driver_id" ] && get_driver_id $driver

    echo "Attempt uninstall of '$driver' from hub '$hub_id'"
    smartthings edge:drivers:uninstall -H $hub_id $driver_id

    echo "Attempt unpublish of '$driver' from channel '$channel'"
    smartthings edge:drivers:unpublish -C $channel_id $driver_id

    echo "Attempt delete of '$driver' package"
    smartthings edge:drivers:delete $driver_id
}

case "$1" in
  setup)
    setup
    ;;
  install)
    install
    ;;
  reset)
    reset
    ;;
  test)
    test
    ;;
  *)
    echo "Unexpected command"
    ;;
esac
