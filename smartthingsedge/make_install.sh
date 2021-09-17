#!/bin/bash
smartthings edge:drivers:package whichswitch
smartthings edge:drivers:publish 2139cec4-d550-4e9c-a4c9-0c16f34ba8c0 -C d89b3b3e-4d58-4381-9da6-be285f04cab6
smartthings edge:drivers:install -H 3795a415-54ae-4621-ac43-463926aba2fa -C d89b3b3e-4d58-4381-9da6-be285f04cab6 2139cec4-d550-4e9c-a4c9-0c16f34ba8c0

# this is meant to show the log output...
#smartthings edge:drivers:logcat --hub-address=192.168.0.177 --all