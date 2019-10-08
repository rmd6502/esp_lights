#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <ESP8266mDNS.h>

#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <WiFiManager.h>

#include "config.h"

// R1,G1,B1,R2,...
uint8_t ledGPIOs[] {5,4,15,13,12,14};

WiFiClient espClient;
PubSubClient client(espClient);
String subscription_key;

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqttUser, mqttPass)) {
      Serial.println("connected");
      // ... and resubscribe
      client.subscribe(subscription_key.c_str());
      return;
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length);
void setup() {
  Serial.begin(115200);
  Serial.println("Starting connecting WiFi.");
  delay(10);
 
  WiFiManager wifiManager;
  wifiManager.setConfigPortalTimeout(180);
  if (!wifiManager.autoConnect(apName, apPass)) {
    Serial.println("failed to connect, we should reset to see if it connects");
    delay(1000);
    ESP.reset();
    delay(5000);
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqttHost, 1883);
  client.setCallback(callback);
  subscription_key = mqttUser;
  subscription_key += "/feeds";
  subscription_key += subscription;
  Serial.println(subscription_key);
 
   ArduinoOTA.setHostname(host);

   ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.begin();

    analogWriteFreq(8000);
    analogWriteRange(255);
}

void loop() {
  ArduinoOTA.handle();
  client.loop();

  if (!client.connected()) {
    reconnect();
  }
}

void setLEDColor(const char *colorstr);
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  char *payloadStr = (char *)malloc(length + 1);
  strncpy(payloadStr, (char *)payload, length);
  payloadStr[length] = '\0';
  setLEDColor(payloadStr);
  free(payloadStr);
}
/* colorstr = RRGGBB in hex */
 void setLEDColor(const char *colorstr) {
  uint32_t r,g,b;
  if (colorstr[0] == '#') {
    sscanf(colorstr, "#%02x%02x%02x", &r, &g, &b);
  } else {
    sscanf(colorstr, "%ld", &r);
    g=b=0;
  }
  
  Serial.printf("SetLED R %d G %d B %d", r, g, b);
  analogWrite(ledGPIOs[0], r);
  analogWrite(ledGPIOs[3], r);
  analogWrite(ledGPIOs[1], g);
  analogWrite(ledGPIOs[4], g);
  analogWrite(ledGPIOs[2], b);
  analogWrite(ledGPIOs[5], b);
}
