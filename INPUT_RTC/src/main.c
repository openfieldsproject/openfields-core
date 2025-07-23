/*
 * Module Name: INPUT_LOCALHOST
 * Description: Collects data localhost(e.g.server , epoch, localtime, cpu storage and memory stats).
 * Author: Gavin Behrens
 * License: GPLv3
 * Dependencies: mqtt_client.h, time.h
 * Notes:
 *   - Publishes to: /DATA/HOST/...
 *   - No AES128-CBC necessary - input module but source is localhost only
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mosquitto.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

#define HOST     "127.0.0.1"
#define PORT     1883
#define KEEPALIVE 60
#define QOS      1
#define MAXCHAR 32

volatile sig_atomic_t running = 1;

//This will just break cleanly
void handle_sigint(int sig)
{
  (void) sig;
  running = 0;
}


int main()
{
    struct mosquitto *mosq = NULL;
    int rc;
    time_t epoch_time = time(NULL);
    struct tm *local_time_info;
    char topic[MAXCHAR];
    char payload[MAXCHAR];

    strncpy(topic, "HOST/SERVER/epoch", MAXCHAR - 1);
    topic[MAXCHAR - 1] = '\0';

    signal(SIGINT,handle_sigint);

    mosquitto_lib_init();

    mosq = mosquitto_new("client-pub", true, NULL);
    if (!mosq) {
        fprintf(stderr, "Failed to create Mosquitto instance\n");
        mosquitto_lib_cleanup();
        return EXIT_FAILURE;
    }

    rc = mosquitto_connect(mosq, HOST, PORT, KEEPALIVE);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Unable to connect: %s\n", mosquitto_strerror(rc));
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        return EXIT_FAILURE;
    }

    mosquitto_loop_start(mosq);

    while (running)
    {
      sleep(1);
      epoch_time = time(NULL);
      local_time_info = localtime(&epoch_time);

      //publish rtc epoch
      strncpy(topic,"HOST/SERVER/rtc_epoch",MAXCHAR-1);
      snprintf(payload,MAXCHAR-1,"%ld",epoch_time);
      rc = mosquitto_publish(mosq, NULL, topic, (int) strlen(payload), payload, QOS, false);
      if (rc != MOSQ_ERR_SUCCESS)
        fprintf(stderr, "Publish failed: %s\n", mosquitto_strerror(rc));


      //publish rtc local date
      strncpy(topic,"HOST/SERVER/local_date",MAXCHAR-1);
      strftime(payload, sizeof(payload), "%d/%m/%Y", local_time_info);
      rc = mosquitto_publish(mosq, NULL, topic, (int) strlen(payload), payload, QOS, false);
      if (rc != MOSQ_ERR_SUCCESS)
        fprintf(stderr, "Publish failed: %s\n", mosquitto_strerror(rc));

      //publish rtc day of the week
      strncpy(topic,"HOST/SERVER/local_dotw",MAXCHAR-1);
      strftime(payload, sizeof(payload), "%a", local_time_info);
      rc = mosquitto_publish(mosq, NULL, topic, (int) strlen(payload), payload, QOS, false);
      if (rc != MOSQ_ERR_SUCCESS)
        fprintf(stderr, "Publish failed: %s\n", mosquitto_strerror(rc));

      //publish rtc day of the year
      strncpy(topic,"HOST/SERVER/local_doty",MAXCHAR-1);
      strftime(payload, sizeof(payload), "%j", local_time_info);
      rc = mosquitto_publish(mosq, NULL, topic, (int) strlen(payload), payload, QOS, false);
      if (rc != MOSQ_ERR_SUCCESS)
        fprintf(stderr, "Publish failed: %s\n", mosquitto_strerror(rc));

      //publish rtc week of the year
      strncpy(topic,"HOST/SERVER/local_woty",MAXCHAR-1);
      strftime(payload, sizeof(payload), "%U", local_time_info);
      rc = mosquitto_publish(mosq, NULL, topic, (int) strlen(payload), payload, QOS, false);
      if (rc != MOSQ_ERR_SUCCESS)
        fprintf(stderr, "Publish failed: %s\n", mosquitto_strerror(rc));

      //publish rtc day of the week
      strncpy(topic,"HOST/SERVER/local_timezone",MAXCHAR-1);
      strftime(payload, sizeof(payload), "%Z", local_time_info);
      rc = mosquitto_publish(mosq, NULL, topic, (int) strlen(payload), payload, QOS, false);
      if (rc != MOSQ_ERR_SUCCESS)
        fprintf(stderr, "Publish failed: %s\n", mosquitto_strerror(rc));

      //publish rtc local 12hr time
      strncpy(topic,"HOST/SERVER/local_time_12",MAXCHAR-1);
      strftime(payload, sizeof(payload), "%r", local_time_info);
      rc = mosquitto_publish(mosq, NULL, topic, (int) strlen(payload), payload, QOS, false);
      if (rc != MOSQ_ERR_SUCCESS)
        fprintf(stderr, "Publish failed: %s\n", mosquitto_strerror(rc));

      //publish rtc local 24hr time
      strncpy(topic,"HOST/SERVER/local_time_24",MAXCHAR-1);
      strftime(payload, sizeof(payload), "%X", local_time_info);
      rc = mosquitto_publish(mosq, NULL, topic, (int) strlen(payload), payload, QOS, false);
      if (rc != MOSQ_ERR_SUCCESS)
        fprintf(stderr, "Publish failed: %s\n", mosquitto_strerror(rc));


    }

    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    return EXIT_SUCCESS;
}





/*
#include <stdio.h>
#include <time.h>

int main() {

    time_t epoch_time = time(NULL);

    struct tm *local_time_info = localtime(&epoch_time);

    char time_string[80];

    // Format the local time into a human-readable string
    // %a: Abbreviated weekday name (e.g., "Wed")
    // %Y: Year with century (e.g., "2023")
    // %m: Month as a decimal number (01-12)
    // %d: Day of the month as a decimal number (01-31)
    // %H: Hour in 24-hour format (00-23)
    // %M: Minute as a decimal number (00-59)
    // %S: Second as a decimal number (00-59)
    // %Z: Time zone name or abbreviation (e.g., "EST", "CST")

    strftime(time_string, sizeof(time_string), "%a %Y-%m-%d %H:%M:%S %Z", local_time_info);

    // Print the formatted local time and date
    printf("Epoch timestamp: %ld\n", epoch_time);
    printf("Local time and date: %s\n", time_string);

    return 0;
}
*/