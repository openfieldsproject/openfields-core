#!/bin/bash

RRD="/opt/ofp/wdata/rrd/fs.rrd"

# Get used space of /opt/ofp/wdata in MB
USED_MB=$(df -m /opt/ofp/wdata | awk 'NR==2 {print $3}')

rrdtool update "$RRD" N:$USED_MB
