#include <Arduino.h>

#define ANALOG_PIN 34

void setup() {
  Serial.begin(9600);
}

void loop() {
  int sensorValue = analogRead(ANALOG_PIN);
  Serial.println(sensorValue);
  delay(1000);
}