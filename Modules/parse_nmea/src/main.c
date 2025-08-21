/*
 * Module Name: parse_nmea
 * Description: Processes raw sensor input into component or normalized data.
 * Author: Gavin Behrens
 * License: GPLv3
 * Dependencies: mqtt_client.h, math.h, transform_logic.h
 * Notes:
 *   - Subscribes to: RAW/NMEA/#
 *   - Publishes to: DATA/GPS/various_topics
 *   - Stateless preferred (uses current values only)
 *   - Minimal dependencies, fixed memory model
 *   - Common use cases: dew point, heat index, pressure trend, evapotranspiration
 */

#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>
#include <signal.h>
#include "base.h"
#include "utils.h"

void handle_sigint(int sig)
{
  (void)sig;
  exit (EXIT_SUCCESS);
}

int main (int argc, char * argv[])
{
  (void)argc;
  (void)argv;

  application_data appdata = {0};

  signal(SIGINT,handle_sigint);

  struct mosquitto * mosq = mosquitto_new(NULL, true, (void *) &appdata);
  if (mosq == NULL)
  {
    log_error ("Mosquitto Client", "Failed memory init.");
    return (EXIT_FAILURE);
  }

  mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_message_callback_set(mosq, on_message);

  int rc = mosquitto_connect(mosq, MQTT_HOST ,1883, 60);
  if (rc != MOSQ_ERR_SUCCESS)
  {
    mosquitto_destroy(mosq);
    log_error("Mosquitto Client","Unable to connect to MQTT Server");
    return (EXIT_FAILURE);
  }

  mosquitto_loop_forever(mosq, 1, 1);
  // Disconnect and cleanup
  mosquitto_disconnect(mosq);
  mosquitto_destroy(mosq);
  mosquitto_lib_cleanup();

  return (EXIT_SUCCESS);
}
