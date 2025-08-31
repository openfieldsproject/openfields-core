#ifndef OFP_H
#define OFP_H

#include <time.h>
#include <stdint.h>

#define BASEDIR "/opt/openfields/wdata"
#define MAXDATA 1024

#define LASTBYTE(size) ((size) - 1) // Instead of sizeof(charvar) -1


typedef struct
{
    char source[128];
    char data[MAXDATA];
    time_t timestamp;
    uint32_t crc;
} ofp_topic;

uint32_t simple_checksum(const char *data);

int decrypt_message(unsigned char *cipher, int cipher_len, unsigned char *iv, char *out, int out_size, unsigned char * aes_key);

#endif