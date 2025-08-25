/*
 * Module Name: input_wifigps
 * Description: TCP Socket to receive encrypted GPS NMEA sentence payloads
 * Author: Gavin Behrens
 * License: GPLv3
 * Dependencies: mosquitto, sys/socket, openssl
 * Notes:
 *   - Publishes to: /RAW/NMEA/gps
 *   - AES128-CBC necessary
 */

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h> // For TCP_KEEP*
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <mosquitto.h>

#define LISTEN_PORT 10001

// ---------- MQTT Callbacks ----------
void on_connect(struct mosquitto *mosq, void *obj, int reason_code)
{
  (void)obj;
  mosquitto_connack_string(reason_code);
  if(reason_code != 0) mosquitto_disconnect(mosq);
}

void on_publish(struct mosquitto *mosq, void *obj, int mid)
{
  (void)mosq; (void)obj; (void)mid;
}

static const unsigned char g_aes_key[16] =
{0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,
 0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff};

// ---------- AES decrypt ----------
static int decrypt_message(unsigned char *cipher, int cipher_len,
  unsigned char *iv, char *out, int out_size)
{
  int len = 0, plain_len = 0;
  EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
  if(!ctx) return -1;

  if(EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, g_aes_key, iv) != 1)
  {
    EVP_CIPHER_CTX_free(ctx);
    return -1;
  }

  if(EVP_DecryptUpdate(ctx, (unsigned char*)out, &len, cipher, cipher_len) != 1)
  {
    EVP_CIPHER_CTX_free(ctx);
    return -1;
  }
  plain_len = len;

  if(EVP_DecryptFinal_ex(ctx, (unsigned char*)out + len, &len) != 1)
  {
    EVP_CIPHER_CTX_free(ctx);
    return -1;
  }
  plain_len += len;

  EVP_CIPHER_CTX_free(ctx);

  if(plain_len >= out_size) return -1;
  out[plain_len] = '\0';
  return plain_len;
}

// ---------- TCP read ----------
ssize_t read_n_bytes(int fd, void *buf, size_t n)
{
  size_t total = 0;
  char *ptr = buf;
  while(total < n)
  {
    ssize_t r = read(fd, ptr + total, n - total);
    if(r <= 0) return r;
    total += (size_t) r;
  }
  return (ssize_t)total;
}

// ---------- Handle single client ----------
void handle_client(int client_fd, struct mosquitto *mosq)
{
  unsigned char iv[16];
  unsigned char buffer[1024];
  char plaintext[1024];

  while(1)
  {
    ssize_t n = read_n_bytes(client_fd, iv, 16);
    if(n <= 0) break;

    uint32_t net_len;
    n = read_n_bytes(client_fd, &net_len, 4);
    if(n <= 0) break;

    uint32_t cipher_len = ntohl(net_len);
    if(cipher_len <= 0 || cipher_len > sizeof(buffer)) break;

    n = read_n_bytes(client_fd, buffer, cipher_len);
    if(n <= 0) break;

    int plain_len = decrypt_message(buffer, (int)cipher_len, iv, plaintext, sizeof(plaintext));
    if(plain_len > 0)
    {
      printf("Received: %s\n", plaintext);
      mosquitto_publish(mosq, NULL, "RAW/NMEA/gps", (int)strlen(plaintext), plaintext, 1, false);
    }
  }

  close(client_fd);
  printf("Client disconnected\n");
}

// ---------- Main ----------
int main()
{
  struct mosquitto *mosq;
  int rc = 0;

  signal(SIGPIPE, SIG_IGN);

  mosquitto_lib_init();
  mosq = mosquitto_new(NULL, true, NULL);
  if(!mosq)
  {
    fprintf(stderr, "Error: Out of memory.\n");
    return (EXIT_FAILURE);
  }

  mosquitto_connect_callback_set(mosq, on_connect);
  mosquitto_publish_callback_set(mosq, on_publish);

  rc = mosquitto_connect(mosq, "127.0.0.1", 1883, 60);
  if(rc != MOSQ_ERR_SUCCESS)
  {
    mosquitto_destroy(mosq);
    fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
    return (EXIT_FAILURE);
  }

  rc = mosquitto_loop_start(mosq);
  if(rc != MOSQ_ERR_SUCCESS)
  {
    mosquitto_destroy(mosq);
    fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
    return (EXIT_FAILURE);
  }

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if(server_fd < 0) { perror("socket"); return 1; }

  int opt = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(LISTEN_PORT);

  if(bind(server_fd,(struct sockaddr*)&addr,sizeof(addr)) < 0) { perror("bind"); return 1; }
  if(listen(server_fd,5) < 0) { perror("listen"); return 1; }

  printf("Server listening on port %d\n", LISTEN_PORT);

  while(1)
  {
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
    if(client_fd >= 0)
    {
      printf("Accepted connection from %s:%d\n",
        inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

      // Enable TCP keepalive
      int keepalive = 1;
      setsockopt(client_fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive));
      int keepidle = 10;
      setsockopt(client_fd, IPPROTO_TCP, TCP_KEEPIDLE, &keepidle, sizeof(keepidle));
      int keepintvl = 5;
      setsockopt(client_fd, IPPROTO_TCP, TCP_KEEPINTVL, &keepintvl, sizeof(keepintvl));
      int keepcnt = 3;
      setsockopt(client_fd, IPPROTO_TCP, TCP_KEEPCNT, &keepcnt, sizeof(keepcnt));

      handle_client(client_fd, mosq);
    }
  }

  close(server_fd);
  mosquitto_loop_stop(mosq, true);
  mosquitto_destroy(mosq);
  mosquitto_lib_cleanup();

  return 0;
}

