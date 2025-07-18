#ifndef __BASE_H_
#define __BASE_H_
#include <stdint.h>
#include <mosquitto.h>

/*MACROS*/
#define MAXBUF 384

/*TYPEDEFS*/

typedef struct
{
    char mqtt_host[64];
    int mqtt_port;
    char tty_port[64];
    int baud_rate;
} config_t;

/*FUNCTION DECLARATIONS*/

int read_config(const char *filename, config_t *config);
speed_t get_baud_enum(int baud);

void on_connect(struct mosquitto *mosq, void *obj, int reason_code);
void on_publish(struct mosquitto *mosq, void *obj, int mid);

#endif
