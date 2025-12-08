#!/bin/bash

TEMP_RAW=$(cat /sys/class/thermal/thermal_zone0/temp)
TEMP_C=$(echo "scale=1; $TEMP_RAW / 1000" | bc)

rrdtool update /opt/ofp/wdata/rrd/cpu_temp.rrd N:$TEMP_C
