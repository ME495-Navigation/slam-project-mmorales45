#!/usr/bin/env bash
export ROS_MASTER_URI=http://marco:11311
source /home/msr/install/setup.bash
exec "$@"