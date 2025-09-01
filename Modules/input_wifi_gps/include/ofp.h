#ifndef OFP_H
#define OFP_H

#include <time.h>
#include <stdint.h>

#define BASEDIR "/opt/openfields/wdata"
#define MAXDATA 1024

#define LASTBYTE (MAXDATA - 1)


typedef struct
{
    char source[128];
    char data[MAXDATA];
    time_t timestamp;
    uint32_t flags;
    uint32_t crc;
} ofp_topic;

void ofp_error(const char *payload);

uint32_t simple_checksum(const char *data);

int decrypt_message(unsigned char *cipher, int cipher_len, unsigned char *iv, char *out, int out_size, unsigned char * aes_key);

#endif