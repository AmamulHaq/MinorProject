#include <Arduino.h>
#define LED_PIN 7        // D7
const int batteryPin = A0;
const float referenceVoltage = 5.0;   // Arduino Uno reference

void setup() {
  Serial.begin(9600);

  pinMode(LED_PIN, OUTPUT);
}

void loop() {

  // Turn LED ON
  digitalWrite(LED_PIN, HIGH);

  // Take battery reading ONLY when D7 is HIGH
  int adcValue = analogRead(batteryPin);
  float batteryVoltage = (adcValue * referenceVoltage * 1.47) / 1023.0;

  Serial.print("LED: ON | ADC Value: ");
  Serial.print(adcValue);
  Serial.print(" | Battery Voltage: ");
  Serial.print(batteryVoltage, 2);
  Serial.println(" V");

  delay(5000);   // LED ON for 5 seconds

  // Turn LED OFF
  digitalWrite(LED_PIN, LOW);
  Serial.println("LED: OFF (No Reading)");

  delay(5000);   // LED OFF for 5 seconds
}
