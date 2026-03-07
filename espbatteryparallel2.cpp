#include <Arduino.h>

#define LED1_PIN 18
#define LED2_PIN 19
#define BATTERY_PIN 2

const float referenceVoltage = 3.3;

void setup() {
  Serial.begin(115200);

  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
}

void loop() {

  // -------- LED1 ON PERIOD --------
  digitalWrite(LED1_PIN, HIGH);
  digitalWrite(LED2_PIN, LOW);

  unsigned long startTime = millis();

  while (millis() - startTime < 5000) {

    int adcValue = analogRead(BATTERY_PIN);
    float batteryVoltage = (adcValue * referenceVoltage) / 4095.0;

    Serial.print("LED1: ON  | LED2: OFF | ADC: ");
    Serial.print(adcValue);
    Serial.print(" | Voltage: ");
    Serial.print(batteryVoltage, 2);
    Serial.println(" V");

    delay(1000);
  }

  // -------- LED2 ON PERIOD --------
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, HIGH);

  startTime = millis();

  while (millis() - startTime < 5000) {

    int adcValue = analogRead(BATTERY_PIN);
    float batteryVoltage = (adcValue * referenceVoltage) / 4095.0;

    Serial.print("LED1: OFF | LED2: ON  | ADC: ");
    Serial.print(adcValue);
    Serial.print(" | Voltage: ");
    Serial.print(batteryVoltage, 2);
    Serial.println(" V");

    delay(1000);
  }
}