#include <Arduino.h>

#define LED1_PIN 7
#define LED2_PIN 8

const int batteryPin = A0;
const float referenceVoltage = 5.0;

void setup() {
  Serial.begin(9600);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
}

void loop() {

  // ---------------- LED1 ON PERIOD ----------------
  digitalWrite(LED1_PIN, HIGH);
  digitalWrite(LED2_PIN, LOW);

  unsigned long startTime = millis();

  while (millis() - startTime < 5000) {

    int adcValue = analogRead(batteryPin);
    float batteryVoltage = (adcValue * referenceVoltage) / 1023.0;

    Serial.print("LED1: ON  | LED2: OFF | ADC: ");
    Serial.print(adcValue);
    Serial.print(" | Voltage: ");
    Serial.print(batteryVoltage, 2);
    Serial.println(" V");

    delay(1000);
  }


  // ---------------- LED2 ON PERIOD ----------------
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, HIGH);

  startTime = millis();

  while (millis() - startTime < 5000) {

    int adcValue = analogRead(batteryPin);
    float batteryVoltage = (adcValue * referenceVoltage) / 1023.0;

    Serial.print("LED1: OFF | LED2: ON  | ADC: ");
    Serial.print(adcValue);
    Serial.print(" | Voltage: ");
    Serial.print(batteryVoltage, 2);
    Serial.println(" V");

    delay(1000);
  }
}