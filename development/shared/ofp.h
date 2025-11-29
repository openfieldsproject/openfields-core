#ifndef __OFP_H
#define __OFP_H


typedef struct ofp
{
  int           version;
  int           epoch;
  char          source[12];
  unsigned char flags;
  unsigned char data[32];
  char          notes[976];
  //
} ofpdata;


int get_ofptime();
int ofptime_quality();
int get_lastsyncdelta();
int ofplog(const char * entry);

#endif