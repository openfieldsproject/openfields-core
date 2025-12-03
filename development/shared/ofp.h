#ifndef __OFP_H
#define __OFP_H


typedef struct ofp
{
  unsigned char version;  // OFP data structure V1.0
  time_t        epoch;
  char          source[33];
  unsigned char data[32];
  char          notes[512];
  unsigned char flags;
  unsigned char crc;
} ofpdata;      

int ofp_read(const char * readfile, ofpdata * ofpdata);
int ofp_write(const char * writefile, ofpdata * ofpdata);
time_t get_ofptime();
time_t ofptime_quality();
time_t get_lastsyncdelta(const char * filename);
unsigned int ofplog(const char * entry);

#endif