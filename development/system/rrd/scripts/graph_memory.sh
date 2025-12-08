#!/bin/bash

RRD_FILE="/opt/ofp/wdata/rrd/memory.rrd"
IMG_PATH="/opt/ofp/wdata/rrd/graphs/"
mkdir -p "$IMG_PATH"

generate_memory_graph() {
    local time_range=$1
    local title=$2
    local output_file=$3

    rrdtool graph "$output_file" \
        --width 800 --height 400 \
        --lower-limit 0 --upper-limit 2048 \
        --vertical-label "Memory (MB)" \
        --units-exponent 0 \
        --title "$title" \
        --start "$time_range" --end now \
        --color BACK#000000 \
        --color CANVAS#000000 \
        --color FONT#FFFFFF \
        --color AXIS#FFFFFF \
        --color GRID#555555 \
        --color MGRID#888888 \
        DEF:used="$RRD_FILE":used:AVERAGE \
        CDEF:green=used,512,LT,used,0,IF \
        AREA:green#00FF00:"Safe (<25%)" \
        CDEF:yellow=used,512,GT,used,0,IF \
        AREA:yellow#FFFF00:"Warning (25-50%)" \
        CDEF:orange=used,1024,GT,used,0,IF \
        AREA:orange#FFA500:"Careful (50-75%)" \
        CDEF:red=used,1536,GT,used,0,IF \
        AREA:red#FF4444:"Critical (>75%)" \
        LINE2:used#FFFFFF:"Used" \
        HRULE:512#00FF00:"Safe (<25%)" \
        HRULE:1024#FFFF00:"Warning (25-50%)" \
        HRULE:1536#FFA500:"Careful (50-75%)" \
        HRULE:2048#FF4444:"Critical (>75%)" 
}

# Generate graphs
generate_memory_graph "-1h"  "Memory Usage (Last 1 Hour)"   "${IMG_PATH}memory_1hour.png"
generate_memory_graph "-24h" "Memory Usage (Last 24 Hours)" "${IMG_PATH}memory_24hour.png"
generate_memory_graph "-1w"  "Memory Usage (Last 1 Week)"   "${IMG_PATH}memory_1week.png"
