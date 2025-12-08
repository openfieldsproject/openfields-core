#!/bin/bash

RRD="/opt/ofp/wdata/rrd/fs.rrd"
OUT="/opt/ofp/wdata/rrd/graphs/"
mkdir -p "$OUT"

generate_fs_graph() {
    local range=$1
    local title=$2
    local file=$3

    rrdtool graph "$file" \
        --width 800 --height 400 \
        --lower-limit 0 --upper-limit 2000 \
        --vertical-label "Used Space (MB)" \
        --title "$title" \
        --start "$range" --end now \
        --color BACK#000000 \
        --color CANVAS#000000 \
        --color FONT#FFFFFF \
        --color AXIS#FFFFFF \
        --color GRID#555555 \
        --color MGRID#888888 \
        DEF:used="$RRD":used:AVERAGE \
        \
        CDEF:safe=used,0,512,LIMIT \
        AREA:safe#00FF00:"Safe (0–512 MB)" \
        \
        CDEF:warn=used,512,1024,LIMIT \
        AREA:warn#FFFF00:"Warning (512–1024 MB)" \
        \
        CDEF:care=used,1024,1536,LIMIT \
        AREA:care#FFA500:"Careful (1–1.5 GB)" \
        \
        CDEF:crit=used,1536,2000,LIMIT \
        AREA:crit#FF4444:"Critical (>1.5 GB)" \
        \
        LINE2:used#FFFFFF:"Used"
}

generate_fs_graph "-24h"  "Filesystem Usage (Last 24 Hours)" "$OUT/fs_24h.png"
generate_fs_graph "-30d"  "Filesystem Usage (Last 30 Days)"  "$OUT/fs_30d.png"
generate_fs_graph "-365d" "Filesystem Usage (Last 1 Year)"   "$OUT/fs_1y.png"
