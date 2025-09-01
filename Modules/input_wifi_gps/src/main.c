/*
 * Module Name: input_wifigps_ramfs
 * Description: TCP Socket to receive encrypted node-filtered GPS NMEA payloads and write to RAMFS
 * Author: Gavin Behrens (modified)
 * License: GPLv3
 * Dependencies: sys/socket, openssl, standard libc
 * Notes:
 *   - Writes to to: /opt/openfields/raw/nmea/gpsX
 *   - External Communications -> AES128-CBC necessary
 *   - Uses RAMFS(handled by OS; configured in /etc/fstab.), therefore normal file handling functions required.
 *
 */

// ---------- Includes ----------
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <stdint.h>
#include "ofp.h"

// ---------- Macros ----------
#define THIS_UNIT   "input_wifi_gps: "
#define LISTEN_PORT 10001
#define SOURCE      "PHYSICAL GPS 1 - tcp://172.18.200.102:10001"
#define TARGET      "/raw/nmea/gps1"

#define AES_KEY ((unsigned char[]){ 0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff })

// ---------- Helper Functions ----------
// --------------------------------------

// ---------- TCP read helper ----------
static ssize_t read_n_bytes(int fd, void *buf, size_t n)
{
  size_t total = 0;
  char *ptr = buf;

  while(total < n)
  {
    ssize_t r = read(fd, ptr + total, n - total);

    if(r <= 0)
      return r;

    total += (size_t) r;
  }

  return (ssize_t)total;
}

// ---------- Write topic to RAMFS ----------
static void publish_ramfs(const char *payload)
{
  ofp_topic msg = {0};
  FILE *f = fopen(BASEDIR TARGET, "wb");

  if (!f)
  {
    perror("fopen");
    ofp_error(THIS_UNIT "Unable to find output file path\n");
    return;
  }

  strncpy(msg.source, SOURCE, sizeof(msg.source)-1);
  msg.source[sizeof(msg.source)-1] = '\0';

  strncpy(msg.data, payload, sizeof(msg.data)-1);
  msg.data[sizeof(msg.data)-1] = '\0';

  msg.timestamp = time(NULL);
  msg.crc = simple_checksum(msg.data);
  msg.flags |= (1<<0);

  fwrite(&msg, sizeof(ofp_topic), 1, f);
  fflush(f);
  fsync(fileno(f));
  fclose(f);
}

// ---------- Handle single client ----------
static void handle_client(int client_fd)
{
  unsigned char iv[16];
  unsigned char buffer[1024];
  char plaintext[1024];

  plaintext[0] = '\0';

  while(1)
  {
    ssize_t n = read_n_bytes(client_fd, iv, 16);
    uint32_t net_len = 0;
    uint32_t cipher_len = 0;
    int plain_len = 0;

    if(n <= 0)
      break;

    n = read_n_bytes(client_fd, &net_len, 4);
    if(n <= 0)
      break;

    cipher_len = ntohl(net_len);
    if(cipher_len == 0 || cipher_len > sizeof(buffer))
      break;

    n = read_n_bytes(client_fd, buffer, cipher_len);
    if(n <= 0)
      break;

    plain_len = decrypt_message(buffer, (int)cipher_len, iv, plaintext, sizeof(plaintext), AES_KEY);
    if(plain_len > 0)
    {
      printf("Received: %s\n", plaintext);
      publish_ramfs(plaintext);
    }
  }

  close(client_fd);
  printf("Client disconnected\n");
}

// ---------- Main Function----------
// ----------------------------------
int main()
{
  signal(SIGPIPE, SIG_IGN);

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if(server_fd < 0)
  {
    perror("socket");
    ofp_error(THIS_UNIT "Unable to creat TCP socket\n");
    return 1;
}

  int opt = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(LISTEN_PORT);

  if(bind(server_fd,(struct sockaddr*)&addr,sizeof(addr)) < 0)
  {
    perror("bind");
    ofp_error(THIS_UNIT "Unable to bind TCP port\n");
    return 1;
  }

  if(listen(server_fd,5) < 0)
  {
    perror("listen");
    ofp_error(THIS_UNIT "Unable to listen on TCP port\n");
    return 1;
  }

  printf("Server listening on port %d\n", LISTEN_PORT);

  while(1)
  {
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
    if(client_fd >= 0)
    {
      printf("Accepted connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

      // Enable TCP keepalive
      int keepalive = 1;
      setsockopt(client_fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive));
      int keepidle = 10;
      setsockopt(client_fd, IPPROTO_TCP, TCP_KEEPIDLE, &keepidle, sizeof(keepidle));
      int keepintvl = 5;
      setsockopt(client_fd, IPPROTO_TCP, TCP_KEEPINTVL, &keepintvl, sizeof(keepintvl));
      int keepcnt = 3;
      setsockopt(client_fd, IPPROTO_TCP, TCP_KEEPCNT, &keepcnt, sizeof(keepcnt));

      // Handles the data from tcp source
      handle_client(client_fd);
    }
  }

  close(server_fd);
  return 0;
}