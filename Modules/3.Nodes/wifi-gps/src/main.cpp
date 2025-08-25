#include <WiFi.h>
#include <WiFiClient.h>
#include "mbedtls/aes.h"

// WiFi credentials
#define WIFI_SSID "SSID"
#define WIFI_PASS "PSK"

// BeagleBone server
#define SERVER_IP   "x.x.x.x"
#define SERVER_PORT 10001

// AES key (16 bytes)
uint8_t aesKey[16] = {0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,
                      0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff};

WiFiClient client;
HardwareSerial &gpsSerial = Serial1; // GPS UART

unsigned long lastReconnectAttempt = 0;
const unsigned long reconnectInterval = 5000; // 5 seconds

void connectWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
      delay(500);
      Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWiFi connected");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());   // <-- show DHCP IP
    } else {
      Serial.println("\nWiFi connect failed, retrying later");
    }
  }
}

void connectServer() {
  if (!client.connected()) {

    if (client.connect(SERVER_IP, SERVER_PORT)) {
      Serial.println("Connected to server");
    } else {
      Serial.println("What we have here is failure to communicate");
    }
  }
}

void sendNMEAMessage(const char* nmea) {
    if (!client.connected()) {
        connectServer();
        if (!client.connected()) return;
    }

    size_t nmeaLen = strlen(nmea);
    size_t paddedLen = ((nmeaLen / 16) + 1) * 16;
    uint8_t plain[128] = {0};
    memcpy(plain, nmea, nmeaLen);
    uint8_t pad = (uint8_t)(paddedLen - nmeaLen);
    for (size_t i = 0; i < pad; i++) plain[nmeaLen + i] = pad;

    // generate IV
    uint8_t iv[16];
    for (size_t i = 0; i < 16; i++) iv[i] = random(0, 256);

    // copy IV for sending (because mbedTLS modifies it)
    uint8_t iv_to_send[16];
    memcpy(iv_to_send, iv, 16);

    // encrypt
    uint8_t cipher[128] = {0};
    mbedtls_aes_context ctx;
    mbedtls_aes_init(&ctx);
    mbedtls_aes_setkey_enc(&ctx, aesKey, 128);
    mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_ENCRYPT, paddedLen, iv, plain, cipher);
    mbedtls_aes_free(&ctx);

    // send IV first
    client.write(iv_to_send, 16);

    // send ciphertext length
    uint32_t clen = htonl(paddedLen);
    client.write((uint8_t*)&clen, 4);

    // send ciphertext
    client.write(cipher, paddedLen);
}

void setup() {
  Serial.begin(115200);

  // GPS on GPIO32 (RX), GPIO33 (TX)
  gpsSerial.begin(9600, SERIAL_8N1, 32, 33);

  connectWiFi();
  connectServer();
}

void loop() {
  // WiFi/server watchdog
  if (millis() - lastReconnectAttempt > reconnectInterval) {
    connectWiFi();
    connectServer();
    lastReconnectAttempt = millis();
  }

  // Read GPS
  while (gpsSerial.available()) {
    String line = gpsSerial.readStringUntil('\n');
    line.trim();
    //Serial.println(line);
    if (line.startsWith("$GPRMC") || line.startsWith("$GPGGA")) {
      String k = line.c_str();
      sendNMEAMessage(line.c_str());
      Serial.println(k);

      delay(5); // small spacing to avoid flooding
    }
  }

  delay(10); // yield time for background tasks
}



