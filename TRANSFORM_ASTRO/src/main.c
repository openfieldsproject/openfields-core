/*
 * Module Name: TRANSFORM_ASTRO
 * Description: Collects RTC Epoch, GPS longitude & latitude then calculates:
 *              -- Moon Phase
 *              -- Moon Illumination
 *              -- Sunrise / Set
 *              -- Solar Azimuth
 *              -- Solar Elevation
 *              -- Solar Energy (clear sky assumption)
 *  * Author: Gavin Behrens
 * License: GPLv3
 * Dependencies: mqtt_client.h, time.h
 * Notes:
 *   - Publishes to: DATA/ASTRO/...
 *   - No AES128-CBC necessary - transform broker & local data.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mosquitto.h>
#include <signal.h>
#include <string.h>
#include "astro.h"

// MQTT Config
#define HOST          "192.168.122.254"
#define PORT          1883
#define KEEPALIVE     60
#define QOS           1

// Data Ready Flags
#define OK_EPOCH      (1 << 0)
#define OK_LAT        (1 << 1)
#define OK_LON        (1 << 2)
#define OK_TZ_OFFSET  (1 << 3)
#define OK_ALL        (OK_EPOCH | OK_LAT | OK_LON | OK_TZ_OFFSET)

volatile int running = 1;

// Signal handler for clean exit
void handle_sigint(int sig)
{
  (void)sig;
  running = 0;
}

// Callback for incoming MQTT messages
void on_message(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *msg)
{
  astro_info *data = (astro_info *)userdata;
  (void)mosq;
  if (!msg->payloadlen)
    return;

  if (strcmp(msg->topic, "DATA/DATE_TIME/rtc_epoch") == 0)
  {
    data->epoch = atol((char *)msg->payload);
    data->ok_pub |= OK_EPOCH;
  }
  else if (strcmp(msg->topic, "DATA/DATE_TIME/rtc_tz_offset") == 0)
  {
    data->ok_pub |= OK_TZ_OFFSET;
    data->tz_offset = atof((char *)msg->payload);
  }
  else if (strcmp(msg->topic, "DATA/GPS/latitude") == 0)
  {
    data->latitude = atof((char *)msg->payload);
    data->ok_pub |= OK_LAT;
  }
  else if (strcmp(msg->topic, "DATA/GPS/longitude") == 0)
  {
    data->longitude = atof((char *)msg->payload);
    data->ok_pub |= OK_LON;
  }
  else if (strcmp(msg->topic, "CONTROL/ASTRO/req_luna") == 0)
  {
    lunar_data(data);
  }
  else if (strcmp(msg->topic, "CONTROL/ASTRO/req_solar") == 0)
  {
    solar_data(data);
  }

}

int main()
{
  struct mosquitto *mosq = NULL;
  int rc;
  astro_info module_data = {0};

  signal(SIGINT, handle_sigint);

  mosquitto_lib_init();

  mosq = mosquitto_new("transform_astro", true, NULL);
  if (!mosq)
  {
    fprintf(stderr, "Failed to create Mosquitto instance\n");
    mosquitto_lib_cleanup();
    return EXIT_FAILURE;
  }

  mosquitto_message_callback_set(mosq, on_message);
  mosquitto_user_data_set(mosq, &module_data);

  rc = mosquitto_connect(mosq, HOST, PORT, KEEPALIVE);
  if (rc != MOSQ_ERR_SUCCESS)
  {
    fprintf(stderr, "Unable to connect: %s\n", mosquitto_strerror(rc));
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return EXIT_FAILURE;
  }

  // Subscribing to required topics
  mosquitto_subscribe(mosq, NULL, "DATA/DATE_TIME/rtc_epoch", QOS);
  mosquitto_subscribe(mosq, NULL, "DATA/DATE_TIME/rtc_tz_offset", QOS);
  mosquitto_subscribe(mosq, NULL, "DATA/GPS/latitude", QOS);
  mosquitto_subscribe(mosq, NULL, "DATA/GPS/longitude", QOS);
  mosquitto_subscribe(mosq, NULL, "CONTROL/ASTRO/req_luna", QOS);
  mosquitto_subscribe(mosq, NULL, "CONTROL/ASTRO/req_sola", QOS);

  // Wait for all required data
  int timeout_ms = 10000; // 10 seconds
  int waited = 0;
  while ((module_data.ok_pub & OK_ALL) != OK_ALL && waited < timeout_ms && running)
  {
      mosquitto_loop(mosq, 100, 1); // process messages
      usleep(100000); // sleep 100ms
      waited += 100;
  }

  // Once all data is received
  if ((module_data.ok_pub & OK_ALL) == OK_ALL)
  {
    if (lunar_data(&module_data) == EXIT_SUCCESS)
    {
      // publish lunar data
      ;
    }
    else
    {
      fprintf(stderr, "Could not calculate lunar data.\n");
    }

    if (solar_data(&module_data) == EXIT_SUCCESS)
    {
      // publish solar data
      ;
    }
    else
    {
        fprintf(stderr, "Could not calculate solar data.\n");
    }
  }
  else
  {
      fprintf(stderr, "Timed out waiting for all input data.\n");
  }

  mosquitto_disconnect(mosq);
  mosquitto_destroy(mosq);
  mosquitto_lib_cleanup();

  return EXIT_SUCCESS;
}