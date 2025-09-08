#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "config.h"

// ---------- Read configuration file ----------
uint8_t read_config(config_t *config)
{
  uint8_t valid_config = 0;
  FILE *file = fopen("/etc/openfields/input/wifigps.conf", "r");
  if (!file)
  {
    printf ("Could not locate configuration file.\n");
    return 1;
  }

  char line[128];
  while (fgets(line, sizeof(line), file))
  {
    char key[64], value[64];
    if (sscanf(line, "%63[^=]=%63s", key, value) == 2)
    {
      if (strcmp(key, "module_name") == 0)
      {
        strncpy(config->module_name, value, MAXDATA - 1);
        config->module_name[MAXDATA-1] = '\0'; // Ensure null-termination
        valid_config |= (1<<0);               // Set bit 0
      }

      else if (strcmp(key, "listen_port") == 0)
      {
        int a = atoi(value);
        if (a < 1 || a > 65535)
          config->listen_port = 10001; // Default port
        else
          config->listen_port = (uint16_t)a;

        valid_config |= (1<<1);               // Set bit 1
      }

      else if (strcmp(key, "source") == 0)
      {
        strncpy(config->source, value, MAXDATA - 1);
        config->source[MAXDATA - 1] = '\0';      // Ensure null-termination
        valid_config |= (1<<2);               // Set bit 2
      }

      else if (strcmp(key, "target") == 0)
      {
        strncpy(config->target, value, MAXDATA - 1);
        config->target[MAXDATA - 1] = '\0';      // Ensure null-termination
        valid_config |= (1<<3);               // Set bit 3
      }

      else if (strcmp(key, "aes_key") == 0)
      {
        strncpy(config->aes_key, value, 16);
        config->aes_key[16] = '\0';          // Ensure null-termination
        valid_config |= (1<<4);              // Set bit 4
      }
    }
  }
  fclose(file);

  if (valid_config == 0b11111) // All required fields are set
  {
    printf ("Valid configuration found.\n");
    return 0; // No faults
  }

  else
  {
    printf ("%d - Invalid configuration, please check.\n", valid_config);
    return 1;
  }
}