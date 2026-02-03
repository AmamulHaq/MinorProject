#include <Arduino.h>
#define LED7_PIN 7   // D7
#define LED8_PIN 8   // D8
#define BATTERY_PIN A0

const float referenceVoltage = 5.0;

void setup() {
  Serial.begin(9600);

  pinMode(LED7_PIN, OUTPUT);
  pinMode(LED8_PIN, OUTPUT);

  // Ensure both OFF at start
  digitalWrite(LED7_PIN, LOW);
  digitalWrite(LED8_PIN, LOW);
}

void loop() {

  /* -------- D7 SEQUENCE -------- */
  digitalWrite(LED8_PIN, LOW);   // Strict safety lock
  digitalWrite(LED7_PIN, HIGH);

  delay(100);  // settle time
  int adc7 = analogRead(BATTERY_PIN);
  float voltage7 = (adc7 * referenceVoltage) / 1023.0;

  Serial.print("D7 ON | ADC: ");
  Serial.print(adc7);
  Serial.print(" | Voltage: ");
  Serial.print(voltage7, 2);
  Serial.println(" V");

  digitalWrite(LED7_PIN, LOW);   // Turn OFF D7

  delay(2000);


  /* -------- D8 SEQUENCE -------- */
  digitalWrite(LED7_PIN, LOW);   // Strict safety lock
  digitalWrite(LED8_PIN, HIGH);

  delay(100);  // settle time
  int adc8 = analogRead(BATTERY_PIN);
  float voltage8 = (adc8 * referenceVoltage) / 1023.0;

  Serial.print("D8 ON | ADC: ");
  Serial.print(adc8);
  Serial.print(" | Voltage: ");
  Serial.print(voltage8, 2);
  Serial.println(" V");

  digitalWrite(LED8_PIN, LOW);   // Turn OFF D8

  delay(2000);
}
