/*
 * Module Name: input_serail_gps
 * Description: Reads raw nmea sentences from USB GPS receiver and publishes to topic.
 * Author: Gavin Behrens
 * License: GPLv3
 * Dependencies:
 * Notes:
 *   - Publishes to: RAW/NMEA/gps
 *   - no AES128-CBC - not sensative information, anyone with gps receiver sees the same data.
 *   - Designed for minimal memory usage, static allocation only
 *   - Reads config file from /etc/openfields/usbgps.conf.
 *   - Staic Analysis with Valgrind.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <stdint.h>
#include <stdlib.h>
#include <mosquitto.h>
#include <signal.h>
#include "base.h"

volatile sig_atomic_t running = 1;

//This will just break cleanly
void handle_sigint(int sig)
{
  (void) sig;
  running = 0;
}


int main(int argc, char *argv[])
{
  (void)argc;   //Config file in use
  (void)argv;
  signal(SIGINT, handle_sigint);

  char buf[MAXBUF] = {0};

  char *p = buf;
  int nbytes = 0;
  ssize_t n;
  config_t app_config = {0};

  struct mosquitto *mosq;
	int rc;

  int fd;
  struct termios options;

  if (read_config("/etc/openfields/usbgps.conf", &app_config) != 0)
  {
    printf ("Can't read config file.\n");
    return (EXIT_FAILURE);
  }

  fd = open(app_config.tty_port, O_RDWR | O_NOCTTY);
  if (fd < 0)
  {
    perror("open");
    return (EXIT_FAILURE);
  }

  tcgetattr(fd, &options);

  speed_t baud = get_baud_enum(app_config.baud_rate);
  if (baud == (speed_t)-1)
  {
    fprintf(stderr, "Unsupported baud rate: %d\n", app_config.baud_rate);
    close(fd);
    return (EXIT_FAILURE);
  }

  cfsetispeed(&options, baud);
  cfsetospeed(&options, baud);

  options.c_cflag |= (CLOCAL | CREAD);
  options.c_cflag &= (tcflag_t)~PARENB;
  options.c_cflag &= (tcflag_t)~CSTOPB;
  options.c_cflag &= (tcflag_t)~CSIZE;
  options.c_cflag |= CS8;
  options.c_lflag &= (tcflag_t)~(ICANON | ECHO | ECHOE | ISIG);
  options.c_oflag &= (tcflag_t)~OPOST;
  options.c_cc[VMIN] = 1;
  options.c_cc[VTIME] = 0;
  tcsetattr(fd, TCSANOW, &options);

  sleep(2);
  tcflush(fd,TCIOFLUSH);

	mosquitto_lib_init();

  mosq = mosquitto_new(NULL, true, NULL);
	if(mosq == NULL)
  {
		fprintf(stderr, "Error: Out of memory.\n");
		return (EXIT_FAILURE);
	}

  mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_publish_callback_set(mosq, on_publish);

  rc = mosquitto_connect(mosq, app_config.mqtt_host, app_config.mqtt_port, 60);
	if(rc != MOSQ_ERR_SUCCESS)
  {
		mosquitto_destroy(mosq);
		fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
		return (EXIT_FAILURE);
	}

	/* Run the network loop in a background thread, this call returns quickly. */
	rc = mosquitto_loop_start(mosq);
	if(rc != MOSQ_ERR_SUCCESS){
		mosquitto_destroy(mosq);
		fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
		return (EXIT_FAILURE);
	}

  while (running)
  {
    n = read(fd, p, 1);
    if (n == -1)
    {
      perror("read");
      break;
    }

    if (n == 0 || *p == '\r')
    {
      continue;
    }

    else if (*p == '\n')
	  {
      *p = '\0';

      if ( strlen(buf) > 3)
      {
        buf[strcspn(buf, "\r\n")] = '\0';  // Clean newline garbage
        mosquitto_publish(mosq, NULL, "RAW/NMEA/gps", (int) strlen(buf), buf, 2, false);
      }

      p = buf;
    nbytes = 0;
    }
    else
    {
      p++;
      nbytes++;
      if (nbytes >= MAXBUF)
		  {
        fprintf(stderr, "Line too long, discarding\n");
        p = buf;
        nbytes = 0;
      }
    }
 }

 close(fd);
 mosquitto_lib_cleanup();

 mosquitto_loop_stop(mosq, true); // force stop, cleanup
 mosquitto_destroy(mosq);

 return (EXIT_SUCCESS);
}



