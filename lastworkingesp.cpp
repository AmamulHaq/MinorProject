#include <Arduino.h>

// Pin definitions for ESP32
#define LED1_PIN        18      // GPIO 18
#define LED2_PIN        19      // GPIO 19
#define BATTERY_PIN     34      // GPIO 34 (ADC1 channel 6, input only)

// Timing constants (in milliseconds)
const unsigned long LED_ON_TIME   = 5000;   // each LED stays on for 5 seconds
const unsigned long SWITCH_DELAY   = 100;   // both LEDs off during switch-over
const unsigned long POST_ON_SETTLE = 100;   // delay after LED on before first reading
const unsigned long MEASURE_INTERVAL = 1000; // time between readings while LED is on

// Voltage reference for ESP32 ADC (3.3V). If your battery voltage is higher,
// you MUST use a voltage divider to bring it down to ≤3.3V.
const float REFERENCE_VOLTAGE = 3.3;
const int ADC_MAX = 4095;                   // 12-bit ADC

void setup() {
  Serial.begin(115200);                     // ESP32 typical baud rate
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  
  // Start with both LEDs off
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);

  // Optional: set ADC attenuation to 0dB for 0-3.3V range (default)
  // analogReadResolution(12); is usually default, but can be set explicitly
  analogReadResolution(12);
}

void loop() {
  // ---------- LED1 ON period ----------
  // Turn off LED2, wait for switch-over settling, then turn on LED1
  digitalWrite(LED2_PIN, LOW);
  delay(SWITCH_DELAY);
  digitalWrite(LED1_PIN, HIGH);
  
  // Allow the battery voltage to settle after the LED turns on
  delay(POST_ON_SETTLE);
  
  // Perform measurements for the next LED_ON_TIME milliseconds
  unsigned long periodStart = millis();        // when the LED was turned on
  unsigned long lastMeasure = periodStart;     // first reading will happen after MEASURE_INTERVAL

  while (millis() - periodStart < LED_ON_TIME) {
    // Take a measurement every MEASURE_INTERVAL
    if (millis() - lastMeasure >= MEASURE_INTERVAL) {
      lastMeasure = millis();

      int adcValue = analogRead(BATTERY_PIN);
      // Convert 12-bit ADC reading to voltage
      float batteryVoltage = (adcValue * REFERENCE_VOLTAGE*2.6) / ADC_MAX;

      Serial.print("LED1: ON  | LED2: OFF | ADC: ");
      Serial.print(adcValue);
      Serial.print(" | Voltage: ");
      Serial.print(batteryVoltage, 2);
      Serial.println(" V");
    }
  }

  // ---------- LED2 ON period ----------
  // Turn off LED1, wait for switch-over settling, then turn on LED2
  digitalWrite(LED1_PIN, LOW);
  delay(SWITCH_DELAY);
  digitalWrite(LED2_PIN, HIGH);
  
  // Allow the battery voltage to settle after the LED turns on
  delay(POST_ON_SETTLE);
  
  periodStart = millis();
  lastMeasure = periodStart;

  while (millis() - periodStart < LED_ON_TIME) {
    if (millis() - lastMeasure >= MEASURE_INTERVAL) {
      lastMeasure = millis();

      int adcValue = analogRead(BATTERY_PIN);
      float batteryVoltage = (adcValue * REFERENCE_VOLTAGE*2.6) / ADC_MAX;

      Serial.print("LED1: OFF | LED2: ON  | ADC: ");
      Serial.print(adcValue);
      Serial.print(" | Voltage: ");
      Serial.print(batteryVoltage, 2);
      Serial.println(" V");
    }
  }
}