#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define ANALOG_PIN 35

// OLED settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// WiFi credentials
const char* ssid = "God Father";
const char* password = "#godfather786";

// Flask server URL
const char* serverUrl = "http://192.168.31.8:5000/insert_voltage";

void setup() {

  Serial.begin(115200);

  // Start OLED I2C
  Wire.begin(21,22);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    Serial.println("OLED not found");
    while(true);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // Connect WiFi
  WiFi.begin(ssid, password);

  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Connecting WiFi...");
  display.display();

  Serial.print("Connecting to WiFi");

  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");

  display.clearDisplay();
  display.setCursor(0,0);
  display.println("WiFi Connected");
  display.display();
  delay(2000);
}

void loop() {

  int sensorValue = analogRead(ANALOG_PIN);

  Serial.print("Analog value: ");
  Serial.println(sensorValue);

  // -------- OLED DISPLAY --------
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("ESP32 Analog Monitor");

  display.setCursor(0,20);
  display.print("ADC Value:");

  display.setTextSize(3);
  display.setCursor(0,35);
  display.println(sensorValue);

  display.display();

  // -------- SEND DATA TO SERVER --------
  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;

    String url = String(serverUrl) + "?voltage=" + String(sensorValue);

    http.begin(url);

    int httpResponseCode = http.GET();

    Serial.print("HTTP Response: ");
    Serial.println(httpResponseCode);

    http.end();
  }
  else {
    Serial.println("WiFi Disconnected");
  }

  delay(5000);
}