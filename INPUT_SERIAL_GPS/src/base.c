#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <mosquitto.h>
#include "base.h"

#define NUMPARTS 32
#define PARTSIZE 32

speed_t get_baud_enum(int baud)
{
  switch (baud)
  {
    case 0: return B0;
    case 50: return B50;
    case 75: return B75;
    case 110: return B110;
    case 134: return B134;
    case 150: return B150;
    case 200: return B200;
    case 300: return B300;
    case 600: return B600;
    case 1200: return B1200;
    case 1800: return B1800;
    case 2400: return B2400;
    case 4800: return B4800;
    case 9600: return B9600;
    case 19200: return B19200;
    case 38400: return B38400;
    case 57600: return B57600;
    case 115200: return B115200;
    case 230400: return B230400;

    default:
      return (speed_t)-1;  // Invalid
    }
}


int read_config(const char *filename, config_t *config)
{
  FILE *file = fopen(filename, "r");
  if (!file) return -1;

  char line[128];
  while (fgets(line, sizeof(line), file))
  {
    char key[64], value[64];
    if (sscanf(line, "%63[^=]=%63s", key, value) == 2)
    {
      if (strcmp(key, "mqtt_host") == 0)
        strncpy(config->mqtt_host, value, sizeof(config->mqtt_host));
      else if (strcmp(key, "mqtt_port") == 0)
        config->mqtt_port = atoi(value);
      else if (strcmp(key, "tty_port") == 0)
        strncpy(config->tty_port, value, sizeof(config->tty_port));
      else if (strcmp(key, "baud_rate") == 0)
        config->baud_rate = atoi(value);
    }
  }
  fclose(file);
  return 0;
}

void on_connect(struct mosquitto *mosq, void *obj, int reason_code)
{
  (void) obj;
  mosquitto_connack_string(reason_code);
  if(reason_code != 0)
  {
    mosquitto_disconnect(mosq);
  }
}

void on_publish(struct mosquitto *mosq, void *obj, int mid)
{
  (void)mosq;
  (void)obj;
  (void)mid;
  return;
}
