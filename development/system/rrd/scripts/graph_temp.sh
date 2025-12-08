#!/bin/bash

RRD_FILE="/opt/ofp/wdata/rrd/cpu_temp.rrd"
IMG_PATH="/opt/ofp/wdata/rrd/graphs/"
mkdir -p "$IMG_PATH"

generate_temp_graph() {
    local time_range=$1
    local title=$2
    local output_file=$3

    rrdtool graph "$output_file" \
        --width 800 --height 400 \
        --lower-limit 0 --upper-limit 100 \
        --vertical-label "CPU Temp (°C)" \
        --title "$title" \
        --start "$time_range" --end now \
        --color BACK#000000 \
        --color CANVAS#000000 \
        --color FONT#FFFFFF \
        --color AXIS#FFFFFF \
        --color GRID#555555 \
        --color MGRID#888888 \
        DEF:temp="$RRD_FILE":temp:AVERAGE \
        CDEF:red=temp,85,GE,temp,0,IF \
        AREA:red#FF4444:"Critical (85+)" \
        CDEF:orange=temp,75,GE,85,LT,temp,0,IF \
        AREA:orange#FFA500:"Careful (75-85°C)" \
        CDEF:yellow=temp,65,GE,75,LT,temp,0,IF \
        AREA:yellow#FFFF00:"Aware (65-75°C)" \
        CDEF:green=temp,65,LT,temp,0,IF \
        AREA:green#00FF00:"Safe (0-65°C)" \
        LINE2:temp#FFFFFF:"CPU Temp" \
        HRULE:65#FFFFFF:"Safe max" \
        HRULE:75#FFFFFF:"Aware max" \
        HRULE:85#FFFFFF:"Careful max"
}

# Generate graphs
generate_temp_graph "-1h"  "CPU Temperature (Last 1 Hour)"   "${IMG_PATH}temp_1hour.png"
generate_temp_graph "-24h" "CPU Temperature (Last 24 Hours)" "${IMG_PATH}temp_24hour.png"
generate_temp_graph "-1w"  "CPU Temperature (Last 1 Week)"   "${IMG_PATH}temp_1week.png"
