#ifndef OFP_H
#define OFP_H

#include <time.h>
#include <stdint.h>

#define MAXDATA 1024

typedef struct
{
    char type[16];
    char data[MAXDATA];
    time_t timestamp;
    uint32_t crc;
} ofp_topic;


#endif