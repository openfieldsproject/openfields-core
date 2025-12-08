#!/bin/bash

RRD_FILE="/opt/ofp/wdata/rrd/memory.rrd"

# Get memory in MB
USED=$(free -m | awk '/Mem:/ {print $3}')
AVAILABLE=$(free -m | awk '/Mem:/ {print $7}')

rrdtool update "$RRD_FILE" N:$USED:$AVAILABLE
