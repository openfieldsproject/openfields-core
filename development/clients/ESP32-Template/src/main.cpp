#include <WiFi.h>

// Set your static IP address details
IPAddress local_IP(192, 168, 223, 201);
IPAddress gateway(192, 168, 223, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);   // Optional
IPAddress secondaryDNS(8, 8, 4, 4); // Optional

const char* ssid = "openfields";
const char* password = "ofpbbdemo";

void setup() 
{
  Serial.begin(115200);

  // Configures static IP
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Just sleeping for now , this is where to ask for time.");
}

void loop() 
{
  sleep(5);  
}