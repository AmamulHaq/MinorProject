/*
 * ESP32 Voltage measurement with non‑linearity correction
 * Uses calibration points to map raw measured voltage to true multimeter voltage.
 */

#define ANALOG_IN_PIN   32
#define REF_VOLTAGE     3.3          // measure your ESP32's 3.3V pin with a multimeter!
#define ADC_RESOLUTION  4096.0
#define R1              30000.0      // replace with actual measured resistance (ohms)
#define R2              7500.0       // replace with actual measured resistance (ohms)

// Calibration points: { measured_voltage, true_multimeter_voltage }
// Add as many points as you have, sorted by measured_voltage.
const float calMeasured[] = { 0.96, 1.70, 2.06, 3.08 };
const float calTrue[]     = { 1.72, 2.52, 2.89, 3.93 };
const int calPoints = sizeof(calMeasured) / sizeof(calMeasured[0]);

float mapVoltage(float measured) {
  // If outside calibration range, use linear extrapolation from nearest segment
  if (measured <= calMeasured[0]) {
    // Extrapolate below first point
    float slope = (calTrue[1] - calTrue[0]) / (calMeasured[1] - calMeasured[0]);
    return calTrue[0] - slope * (calMeasured[0] - measured);
  }
  if (measured >= calMeasured[calPoints-1]) {
    // Extrapolate above last point
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
  return measured; // fallback (should not happen)
}

void setup() {
  Serial.begin(9600);
  analogSetAttenuation(ADC_11db);   // 0‑3.3V range
}

void loop() {
  int adc_value = analogRead(ANALOG_IN_PIN);
  float voltage_adc = (adc_value * REF_VOLTAGE) / ADC_RESOLUTION;
  float raw_voltage_in = voltage_adc * (R1 + R2) / R2;   // raw, uncorrected
  float corrected_voltage = mapVoltage(raw_voltage_in);

  Serial.print("Raw measured = ");
  Serial.print(raw_voltage_in, 2);
  Serial.print(" V  ->  Corrected = ");
  Serial.print(corrected_voltage, 2);
  Serial.println(" V");

  delay(500);
}