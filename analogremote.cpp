#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino.h>
#define ANALOG_PIN 35

// Your WiFi credentials
const char* ssid = "God Father";          // replace with your WiFi name
const char* password = "#godfather786";   // replace with your WiFi password

// Flask server URL (use your PC LAN IP, not localhost)
const char* serverUrl = "http://192.168.31.8:5000/insert_voltage";

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
}

void loop() {
  int sensorValue = analogRead(ANALOG_PIN);
  Serial.print("Analog value: ");
  Serial.println(sensorValue);  // see the value in Serial Monitor

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Send the value to Flask server
    String url = String(serverUrl) + "?voltage=" + String(sensorValue);
    http.begin(url);

    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }

  delay(5000);  // send every 5 seconds
}