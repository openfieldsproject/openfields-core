#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include "ofp.h"

// ---------- Error Loging ----------
void ofp_error(const char *payload)
{
  FILE *f = fopen(BASEDIR "/error.log", "ab");

  if (!f)
  {
    perror("fopen");
    return;
  }

  fwrite(payload, sizeof(char), strlen(payload), f);
  fflush(f);
  fsync(fileno(f));
  fclose(f);
}

// ---------- Simple checksum ----------
uint32_t simple_checksum(const char *data)
{
    uint32_t sum = 0;
    while (*data) sum += (unsigned char)(*data++);
    return sum;
}

// ---------- AES decrypt ----------
int decrypt_message(unsigned char *cipher, int cipher_len,
                           unsigned char *iv, char *out, int out_size, unsigned char * aes_key)
{
    int len = 0, plain_len = 0;
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if(!ctx) return -1;

    if(EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, aes_key, iv) != 1)
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