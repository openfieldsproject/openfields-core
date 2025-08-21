#include <mosquitto.h>
#include <stdio.h>
#include <string.h>
#include "base.h"
#include "utils.h"


void on_connect(struct mosquitto *mosq, void *obj, int reason_code)
{
  (void)obj;
	int rc;

	if(reason_code != 0)
  {
		mosquitto_disconnect(mosq);
	}

	rc = mosquitto_subscribe(mosq, NULL, "RAW/NMEA/#", 1);
	if(rc != MOSQ_ERR_SUCCESS)
  {
		fprintf(stderr, "Error subscribing: %s\n", mosquitto_strerror(rc));

		mosquitto_disconnect(mosq);
	}
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
  int rc;
  application_data * app_data = (application_data *) obj;
  strncpy (app_data->incoming_message,msg->payload,MAXBUF-1);
  parse_message(app_data);

  if (ADMQ != -1)
  { //publish
    for (int8_t i = 0 ; i <= ADMQ; i++)
    {
      rc = mosquitto_publish(mosq, NULL, app_data->parsed_data[i].pub_topic, (int) strlen(app_data->parsed_data[i].pub_payload), app_data->parsed_data[i].pub_payload, 2, app_data->parsed_data[i].pub_persist);
	    if(rc != MOSQ_ERR_SUCCESS)
      {
		    fprintf(stderr, "Error publishing: %s\n", mosquitto_strerror(rc));
	    }
      //printf (" %s   %s  %d\n",app_data->parsed_data[i].pub_topic,app_data->parsed_data[i].pub_payload,app_data->parsed_data[i].pub_persist);
    }
  }
  else return;
}