#include <Arduino.h>
#define ANALOG_PIN 34

void setup() {
  Serial.begin(9600);   // Serial communication
}

void loop() {
  int sensorValue = analogRead(ANALOG_PIN); // read analog input

  Serial.println(sensorValue); // send value to computer

  delay(1000); // small delay for stability
} 