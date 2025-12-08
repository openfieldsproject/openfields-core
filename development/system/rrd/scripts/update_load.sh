#!/bin/bash

# Define the location of your RRD file
RRD_FILE="/opt/ofp/wdata/rrd/loadavg.rrd"

# Extract the first three fields (1min, 5min, 15min averages) from /proc/loadavg
LOAD_AVG=$(cat /proc/loadavg | awk '{print $1 ":" $2 ":" $3}')

# Update the RRD database. 'N' means 'now' for the timestamp.
rrdtool update "$RRD_FILE" N:$LOAD_AVG
