#!/bin/bash

RRD_FILE="/opt/ofp/wdata/rrd/loadavg.rrd"
IMG_PATH="/opt/ofp/wdata/rrd/graphs/"
mkdir -p "$IMG_PATH"

# Auto-detect CPU cores
CORES=$(nproc)   # Fully portable on Linux

generate_graph() {
    local time_range=$1
    local title=$2
    local output_file=$3

    rrdtool graph "$output_file" \
        --width 800 --height 400 \
        --lower-limit 0 \
        --upper-limit 100 \
        --vertical-label "CPU Load (%)" \
        --title "$title" \
        --start "$time_range" --end now \
        --color BACK#000000 \
        --color CANVAS#000000 \
        --color FONT#FFFFFF \
        --color AXIS#FFFFFF \
        --color GRID#555555 \
        --color MGRID#888888 \
        \
        DEF:avg1=$RRD_FILE:load1min:AVERAGE \
        DEF:avg5=$RRD_FILE:load5min:AVERAGE \
        DEF:avg15=$RRD_FILE:load15min:AVERAGE \
        \
        CDEF:pct1=avg1,$CORES,/,100,* \
        CDEF:pct5=avg5,$CORES,/,100,* \
        CDEF:pct15=avg15,$CORES,/,100,* \
        \
        AREA:pct1#FF000030:"1 min" \
        AREA:pct5#00FF0030:"5 min" \
        AREA:pct15#00FFFF30:"15 min" \
        LINE2:pct1#FF0000:"1 min" \
        LINE2:pct5#00FF00:"5 min" \
        LINE2:pct15#00FFFF:"15 min" \
        \
        HRULE:25#FFFFFF:"25%" \
        HRULE:50#CCCCCC:"50%" \
        HRULE:75#AAAAAA:"75%" \
        HRULE:100#FF4444:"100%"
}

generate_graph "-1h"  "CPU Load (Last 1 Hour)"   "${IMG_PATH}load_1hour.png"
generate_graph "-24h" "CPU Load (Last 24 Hours)" "${IMG_PATH}load_24hour.png"
generate_graph "-1w"  "CPU Load (Last 1 Week)"   "${IMG_PATH}load_1week.png"
