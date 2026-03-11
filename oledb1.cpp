
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define ANALOG_PIN 35

void setup() {

  Serial.begin(115200);

  // Start I2C
  Wire.begin(21,22);

  // Initialize OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found");
    while(true);
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0,10);
  display.println("ESP32 ADC");
  display.display();

  delay(2000);
}

void loop() {

  int sensorValue = analogRead(ANALOG_PIN);

  Serial.print("Analog Value: ");
  Serial.println(sensorValue);

  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Analog Value");

  display.setTextSize(3);
  display.setCursor(10,25);
  display.println(sensorValue);

  display.display();

  delay(1000);
}