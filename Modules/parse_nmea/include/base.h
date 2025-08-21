#ifndef __BASE_H_
#define __BASE_H_
#include <stdint.h>
#include <mosquitto.h>

/*MACROS*/
#define MAXBUF      256
#define MAXQUEUE    1024
#define MQTT_HOST   "127.0.0.1"

#define ADMQ app_data->message_queue

/*TYPEDEFS*/

typedef struct
{
  char pub_topic[MAXBUF];
  char pub_payload[MAXBUF];
  uint8_t pub_persist;
} mqtt_packet;


typedef struct
{
  mqtt_packet parsed_data[MAXQUEUE];
  int8_t message_queue;
  char incoming_message[MAXBUF];
}application_data;

/*FUNCTION DECLARATIONS*/

void on_connect(struct mosquitto *mosq, void *obj, int reason_code);
void on_publish(struct mosquitto *mosq, void *obj, int mid);
void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg);

void parse_message (application_data * data);

#endif
