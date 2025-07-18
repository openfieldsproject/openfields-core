#ifndef __SPLITTER_H_
#define __SPLITTER_H_

#include <stdint.h>

#define MAXFIELDS 32
#define MAXSPLIT 32

typedef char field[MAXSPLIT];

typedef struct
{
  uint8_t numfields;
  field fields[MAXFIELDS];
}split_st;

void strsplitz(char delim, char * incoming, split_st * data);

#endif