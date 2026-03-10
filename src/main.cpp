#include <Arduino.h>

#define ANALOG_PIN 35

void setup() {
  Serial.begin(9600);
}

void loop() {
  int sensorValue = analogRead(ANALOG_PIN);
  Serial.println(sensorValue);
  delay(5000);
}