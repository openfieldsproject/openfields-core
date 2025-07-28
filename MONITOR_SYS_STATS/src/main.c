/*
 * Module Name: MONITOR_SYS_STATS
 * Description: Collects the server's CPU , Memory and Storage details
 *  * Author: Gavin Behrens
 * License: GPLv3
 * Dependencies: mqtt_client.h, sysinfo.h statvfs.h
 * Notes:
 *   - Publishes to: MONITOR/SERVER/...
 *   - No AES128-CBC necessary - input module but source is all localhost data only.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mosquitto.h>
#include <signal.h>
#include <string.h>
#include "sys_stat.h"

#define HOST     "127.0.0.1"
#define PORT     1883
#define KEEPALIVE 60
#define QOS      1


//This will just break cleanly
void handle_sigint(int sig)
{
  (void) sig;
}

//main function
int main()
{
  struct mosquitto *mosq = NULL;
  int rc;
  sys_info module_data = {0};

  signal(SIGINT,handle_sigint);

  mosquitto_lib_init();
  mosq = mosquitto_new("sysinfo", true, NULL);
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


  module_data.ok_pub = 0;
  get_memory_info(&module_data);
  get_cpu_load(&module_data);
  get_disk_usage(&module_data);
  get_wifi_signal(&module_data);

  if (module_data.ok_pub == 15)
{
    rc = mosquitto_publish(mosq, NULL, "MONITOR/SERVER/mem_total_MB",
      (int) strlen(module_data.mem_total), module_data.mem_total, QOS, false);
    if (rc != MOSQ_ERR_SUCCESS)
      fprintf(stderr, "Publish failed: %s\n", mosquitto_strerror(rc));

    rc = mosquitto_publish(mosq, NULL, "MONITOR/SERVER/mem_free_MB",
      (int) strlen(module_data.mem_free), module_data.mem_free, QOS, false);
    if (rc != MOSQ_ERR_SUCCESS)
      fprintf(stderr, "Publish failed: %s\n", mosquitto_strerror(rc));

    rc = mosquitto_publish(mosq, NULL, "MONITOR/SERVER/mem_available_MB",
      (int) strlen(module_data.mem_avail), module_data.mem_avail, QOS, false);
    if (rc != MOSQ_ERR_SUCCESS)
      fprintf(stderr, "Publish failed: %s\n", mosquitto_strerror(rc));

    rc = mosquitto_publish(mosq, NULL, "MONITOR/SERVER/mem_buffers_MB",
      (int) strlen(module_data.mem_buffers), module_data.mem_buffers, QOS, false);
    if (rc != MOSQ_ERR_SUCCESS)
      fprintf(stderr, "Publish failed: %s\n", mosquitto_strerror(rc));

    rc = mosquitto_publish(mosq, NULL, "MONITOR/SERVER/mem_cached_MB",
      (int) strlen(module_data.mem_cached), module_data.mem_cached, QOS, false);
    if (rc != MOSQ_ERR_SUCCESS)
      fprintf(stderr, "Publish failed: %s\n", mosquitto_strerror(rc));

    rc = mosquitto_publish(mosq, NULL, "MONITOR/SERVER/disk_total_MB",
      (int) strlen(module_data.disk_total), module_data.disk_total, QOS, false);
    if (rc != MOSQ_ERR_SUCCESS)
      fprintf(stderr, "Publish failed: %s\n", mosquitto_strerror(rc));

    rc = mosquitto_publish(mosq, NULL, "MONITOR/SERVER/disk_used_MB",
      (int) strlen(module_data.disk_used), module_data.disk_used, QOS, false);
    if (rc != MOSQ_ERR_SUCCESS)
      fprintf(stderr, "Publish failed: %s\n", mosquitto_strerror(rc));

    rc = mosquitto_publish(mosq, NULL, "MONITOR/SERVER/disk_free_MB",
      (int) strlen(module_data.disk_free), module_data.disk_free, QOS, false);
    if (rc != MOSQ_ERR_SUCCESS)
      fprintf(stderr, "Publish failed: %s\n", mosquitto_strerror(rc));

    rc = mosquitto_publish(mosq, NULL, "MONITOR/SERVER/cpu_1_min",
      (int) strlen(module_data.cpu_1), module_data.cpu_1, QOS, false);
    if (rc != MOSQ_ERR_SUCCESS)
      fprintf(stderr, "Publish failed: %s\n", mosquitto_strerror(rc));

    rc = mosquitto_publish(mosq, NULL, "MONITOR/SERVER/cpu_5_min",
      (int) strlen(module_data.cpu_5), module_data.cpu_5, QOS, false);
    if (rc != MOSQ_ERR_SUCCESS)
      fprintf(stderr, "Publish failed: %s\n", mosquitto_strerror(rc));

    rc = mosquitto_publish(mosq, NULL, "MONITOR/SERVER/cpu_15_min",
      (int) strlen(module_data.cpu_15), module_data.cpu_15, QOS, false);
    if (rc != MOSQ_ERR_SUCCESS)
      fprintf(stderr, "Publish failed: %s\n", mosquitto_strerror(rc));

    rc = mosquitto_publish(mosq, NULL, "MONITOR/SERVER/wifi_signal_dBm",
      (int) strlen(module_data.wifi_sig_strength), module_data.wifi_sig_strength, QOS, false);
    if (rc != MOSQ_ERR_SUCCESS)
      fprintf(stderr, "Publish failed: %s\n", mosquitto_strerror(rc));

    // Give the client time to send messages (loop manually)
    for (int i = 0; i < 10; ++i) {
        mosquitto_loop(mosq, 100, 1);  // 100ms timeout
        usleep(100000);                // Sleep 100ms
    }
}


  mosquitto_disconnect(mosq);
  mosquitto_destroy(mosq);
  mosquitto_lib_cleanup();

  return EXIT_SUCCESS;
}
