#include <Arduino.h>

// ========== Pin Definitions ==========
#define LED1_PIN        25
#define LED2_PIN        26
#define BATTERY_PIN     32      // GPIO 34 (ADC1) – voltage divider output

// ========== Timing Constants (milliseconds) ==========
const unsigned long LED_ON_TIME     = 5000;    // each LED stays on for 5 sec
const unsigned long SWITCH_DELAY    = 100;     // both LEDs off during switch‑over
const unsigned long POST_ON_SETTLE  = 100;     // delay after LED on before first reading
const unsigned long MEASURE_INTERVAL = 1000;   // interval between readings while LED is on

// ========== Voltage Divider & ADC Settings ==========
const float REF_VOLTAGE   = 3.3;               // ESP32's internal reference (measure it!)
const float ADC_MAX       = 4095.0;            // 12‑bit ADC (0‑4095)
const float R1            = 30000.0;           // top resistor (actual measured value)
const float R2            = 7500.0;            // bottom resistor (actual measured value)

// ========== Calibration Mapping (raw → true voltage) ==========
const float calMeasured[] = { 0.96, 1.70, 2.06, 3.08 };   // raw voltage (after divider)
const float calTrue[]     = { 1.72, 2.52, 2.89, 3.93 };   // true multimeter voltage
const int calPoints = sizeof(calMeasured) / sizeof(calMeasured[0]);

float mapVoltage(float measured) {
  // Extrapolate below first point
  if (measured <= calMeasured[0]) {
    float slope = (calTrue[1] - calTrue[0]) / (calMeasured[1] - calMeasured[0]);
    return calTrue[0] - slope * (calMeasured[0] - measured);
  }
  // Extrapolate above last point
  if (measured >= calMeasured[calPoints-1]) {
    float slope = (calTrue[calPoints-1] - calTrue[calPoints-2]) /
                  (calMeasured[calPoints-1] - calMeasured[calPoints-2]);
    return calTrue[calPoints-1] + slope * (measured - calMeasured[calPoints-1]);
  }
  // Interpolate between calibration points
  for (int i = 0; i < calPoints-1; i++) {
    if (measured >= calMeasured[i] && measured <= calMeasured[i+1]) {
      float t = (measured - calMeasured[i]) / (calMeasured[i+1] - calMeasured[i]);
      return calTrue[i] + t * (calTrue[i+1] - calTrue[i]);
    }
  }
  return measured;  // fallback (should never happen)
}

void setup() {
  Serial.begin(115200);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);

  // Configure ADC for 0‑3.3V input range (11 dB attenuation)
  analogSetAttenuation(ADC_11db);
  analogReadResolution(12);
}

void loop() {
  // ---------- LED1 ON ----------
  digitalWrite(LED2_PIN, LOW);
  delay(SWITCH_DELAY);
  digitalWrite(LED1_PIN, HIGH);
  delay(POST_ON_SETTLE);

  unsigned long periodStart = millis();
  unsigned long lastMeasure = periodStart;

  while (millis() - periodStart < LED_ON_TIME) {
    if (millis() - lastMeasure >= MEASURE_INTERVAL) {
      lastMeasure = millis();

      int adcValue = analogRead(BATTERY_PIN);
      float voltage_at_adc = (adcValue * REF_VOLTAGE) / ADC_MAX;
      float raw_battery_voltage = voltage_at_adc * (R1 + R2) / R2;   // before divider
      float corrected_voltage = mapVoltage(raw_battery_voltage);

      Serial.print("LED1: ON  | LED2: OFF | ADC: ");
      Serial.print(adcValue);
      Serial.print(" | Raw divider out: ");
      Serial.print(voltage_at_adc, 3);
      Serial.print(" V | Battery (raw): ");
      Serial.print(raw_battery_voltage, 2);
      Serial.print(" V | Corrected: ");
      Serial.print(corrected_voltage, 2);
      Serial.println(" V");
    }
  }

  // ---------- LED2 ON ----------
  digitalWrite(LED1_PIN, LOW);
  delay(SWITCH_DELAY);
  digitalWrite(LED2_PIN, HIGH);
  delay(POST_ON_SETTLE);

  periodStart = millis();
  lastMeasure = periodStart;

  while (millis() - periodStart < LED_ON_TIME) {
    if (millis() - lastMeasure >= MEASURE_INTERVAL) {
      lastMeasure = millis();

      int adcValue = analogRead(BATTERY_PIN);
      float voltage_at_adc = (adcValue * REF_VOLTAGE) / ADC_MAX;
      float raw_battery_voltage = voltage_at_adc * (R1 + R2) / R2;
      float corrected_voltage = mapVoltage(raw_battery_voltage);

      Serial.print("LED1: OFF | LED2: ON  | ADC: ");
      Serial.print(adcValue);
      Serial.print(" | Raw divider out: ");
      Serial.print(voltage_at_adc, 3);
      Serial.print(" V | Battery (raw): ");
      Serial.print(raw_battery_voltage, 2);
      Serial.print(" V | Corrected: ");
      Serial.print(corrected_voltage, 2);
      Serial.println(" V");
    }
  }
}