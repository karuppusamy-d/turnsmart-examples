#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiMulti.h>

#include "secrets.h"

#define LED 2

// Allocate the JSON document
StaticJsonDocument<200> doc;
WiFiMulti wifiMulti;

String url = SERVER_URL;
String secret = SECRET;
String projectId = PROJECT_ID;
int ledChannel = 0;

void setup() {
  // Create a Led channel
  ledcSetup(ledChannel, 5000, 8);
  // Attach the channel to the LED pin
  ledcAttachPin(LED, ledChannel);
  // Set the led value to 0
  ledcWrite(ledChannel, 0);

  Serial.begin(115200);  // Starts the serial communication

  // Connecting to Internet
  Serial.println("Setting things up...");

  for (int t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
}

void loop() {
  // wait for WiFi connection
  if ((wifiMulti.run() == WL_CONNECTED)) {
    HTTPClient http;

    Serial.println("[HTTPS] begin...");
    // configure traged server and url
    http.begin(url, root_ca);  // HTTP

    Serial.println("[HTTPS] GET...");

    // start connection and send HTTP data
    String data =
        "{\"uid\": \"" + projectId + "\",\"secret\": \"" + secret + "\"}";
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.sendRequest("GET", data);

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

      // If the response code is 200
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.printf("[HTTPS] Payload: %s\n", payload);

        DeserializationError error = deserializeJson(doc, payload);
        if (!error) {
          boolean onOff = doc["data"]["on"];
          int brightness = doc["data"]["brightness"];

          if (onOff) {
            ledcWrite(ledChannel, 255 * brightness / 100);
          } else {
            ledcWrite(ledChannel, 0);
          }
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", http.getString());
      }
    } else {
      Serial.printf("[HTTPS] GET... failed, error: %s\n",
                    http.errorToString(httpCode).c_str());
    }

    http.end();
  } else {
    Serial.println("[WIFI] Failed to connect to WiFi");
  }
  delay(1000);
}
