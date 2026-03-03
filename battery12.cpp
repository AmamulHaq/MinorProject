#include <Arduino.h>

#define LED_PIN 7
const int batteryPin = A0;
const float referenceVoltage = 5.0;

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
}

void loop() {

  // ---------------- LED ON PERIOD ----------------
  digitalWrite(LED_PIN, HIGH);
  unsigned long startTime = millis();

  while (millis() - startTime < 5000) {   // 5 seconds ON

    int adcValue = analogRead(batteryPin);
    float batteryVoltage = (adcValue * referenceVoltage) / 1023.0;

    Serial.print("LED: ON  | ADC: ");
    Serial.print(adcValue);
    Serial.print(" | Voltage: ");
    Serial.print(batteryVoltage, 2);
    Serial.println(" V");

    delay(1000);  // reading every 1 second
  }


  // ---------------- LED OFF PERIOD ----------------
  digitalWrite(LED_PIN, LOW);
  startTime = millis();

  while (millis() - startTime < 5000) {   // 5 seconds OFF

    int adcValue = analogRead(batteryPin);
    float batteryVoltage = (adcValue * referenceVoltage) / 1023.0;

    Serial.print("LED: OFF | ADC: ");
    Serial.print(adcValue);
    Serial.print(" | Voltage: ");
    Serial.print(batteryVoltage, 2);
    Serial.println(" V");

    delay(1000);  // reading every 1 second
  }
}