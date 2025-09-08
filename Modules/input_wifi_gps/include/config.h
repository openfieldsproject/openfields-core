#ifndef CONFIG_H
#define CONFIG_H
#include <stdint.h>
#include "ofp.h"

/*TYPEDEFS*/

typedef struct
{
  char module_name[MAXDATA];
  uint16_t listen_port;
  char source[MAXDATA];
  char target[MAXDATA];
  char aes_key[17]; // 16 bytes + null terminator
} config_t;

/*FUNCTION DECLARATIONS*/

uint8_t read_config(config_t *config);

#endif