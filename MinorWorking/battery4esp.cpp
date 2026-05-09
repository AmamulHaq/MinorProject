#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ========== OLED Settings ==========
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ========== Pin Definitions ==========
#define LED1_PIN      14
#define LED2_PIN      27
#define LED3_PIN      26
#define LED4_PIN      25
#define BATTERY_PIN   32            // GPIO 34 (ADC1)

// ========== Timing Constants (ms) ==========
const unsigned long LED_ON_TIME     = 5000;   // each LED on for 5 seconds
const unsigned long MEASURE_INTERVAL = 1000;  // take a reading every second
const unsigned long SWITCH_DELAY    = 100;    // both LEDs off during switch
const unsigned long POST_ON_SETTLE  = 100;    // settling after LED turns on

// ========== Voltage Divider & ADC ==========
const float REF_VOLTAGE   = 3.3;              // measure your ESP32's 3.3V pin!
const float ADC_MAX       = 4095.0;
const float R1            = 30000.0;          // top resistor (actual measured value)
const float R2            = 7500.0;           // bottom resistor

// ========== Calibration Mapping ==========
// Format: { raw_battery_voltage (after divider, before mapping), true_multimeter_voltage }
const float calMeasured[] = { 0.96, 1.70, 2.06, 3.08 };
const float calTrue[]     = { 1.72, 2.52, 2.89, 3.93 };
const int calPoints = sizeof(calMeasured) / sizeof(calMeasured[0]);

float mapVoltage(float measured) {
  if (measured <= calMeasured[0]) {
    float slope = (calTrue[1] - calTrue[0]) / (calMeasured[1] - calMeasured[0]);
    return calTrue[0] - slope * (calMeasured[0] - measured);
  }
  if (measured >= calMeasured[calPoints-1]) {
    float slope = (calTrue[calPoints-1] - calTrue[calPoints-2]) /
                  (calMeasured[calPoints-1] - calMeasured[calPoints-2]);
    return calTrue[calPoints-1] + slope * (measured - calMeasured[calPoints-1]);
  }
  for (int i = 0; i < calPoints-1; i++) {
    if (measured >= calMeasured[i] && measured <= calMeasured[i+1]) {
      float t = (measured - calMeasured[i]) / (calMeasured[i+1] - calMeasured[i]);
      return calTrue[i] + t * (calTrue[i+1] - calTrue[i]);
    }
  }
  return measured;
}

// ========== Helper: Read and correct voltage ==========
void readVoltage(int &adcVal, float &correctedVoltage) {
  adcVal = analogRead(BATTERY_PIN);
  float voltage_at_adc = (adcVal * REF_VOLTAGE) / ADC_MAX;
  float raw_battery = voltage_at_adc * (R1 + R2) / R2;
  correctedVoltage = mapVoltage(raw_battery);
}

// ========== Helper: Update OLED display ==========
void showData(String led1, String led2, String led3, String led4, int adcValue, float voltage) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("LED1: "); display.println(led1);
  display.print("LED2: "); display.println(led2);
  display.print("LED3: "); display.println(led3);
  display.print("LED4: "); display.println(led4);
  display.print("ADC: "); display.println(adcValue);
  display.print("Battery: ");
  display.print(voltage, 2);
  display.println(" V");
  display.display();
}

// ========== Setup ==========
void setup() {
  Serial.begin(115200);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  pinMode(LED4_PIN, OUTPUT);
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  digitalWrite(LED3_PIN, LOW);
  digitalWrite(LED4_PIN, LOW);

  analogSetAttenuation(ADC_11db);
  analogReadResolution(12);

  Wire.begin(21, 22);   // I2C pins for OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found");
    while (true);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
}

// ========== Main Loop – Cycle through 4 LEDs ==========
void loop() {
  // ---- LED 1 ON ----
  digitalWrite(LED2_PIN, LOW);
  digitalWrite(LED3_PIN, LOW);
  digitalWrite(LED4_PIN, LOW);
  delay(SWITCH_DELAY);
  digitalWrite(LED1_PIN, HIGH);
  delay(POST_ON_SETTLE);

  unsigned long startTime = millis();
  unsigned long lastMeasure = startTime;
  while (millis() - startTime < LED_ON_TIME) {
    if (millis() - lastMeasure >= MEASURE_INTERVAL) {
      lastMeasure = millis();
      int adcVal;
      float batVoltage;
      readVoltage(adcVal, batVoltage);
      showData("ON", "OFF", "OFF", "OFF", adcVal, batVoltage);
      Serial.printf("LED1 ON | ADC: %d | Battery: %.2f V\n", adcVal, batVoltage);
    }
  }

  // ---- LED 2 ON ----
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED3_PIN, LOW);
  digitalWrite(LED4_PIN, LOW);
  delay(SWITCH_DELAY);
  digitalWrite(LED2_PIN, HIGH);
  delay(POST_ON_SETTLE);

  startTime = millis();
  lastMeasure = startTime;
  while (millis() - startTime < LED_ON_TIME) {
    if (millis() - lastMeasure >= MEASURE_INTERVAL) {
      lastMeasure = millis();
      int adcVal;
      float batVoltage;
      readVoltage(adcVal, batVoltage);
      showData("OFF", "ON", "OFF", "OFF", adcVal, batVoltage);
      Serial.printf("LED2 ON | ADC: %d | Battery: %.2f V\n", adcVal, batVoltage);
    }
  }

  // ---- LED 3 ON ----
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  digitalWrite(LED4_PIN, LOW);
  delay(SWITCH_DELAY);
  digitalWrite(LED3_PIN, HIGH);
  delay(POST_ON_SETTLE);

  startTime = millis();
  lastMeasure = startTime;
  while (millis() - startTime < LED_ON_TIME) {
    if (millis() - lastMeasure >= MEASURE_INTERVAL) {
      lastMeasure = millis();
      int adcVal;
      float batVoltage;
      readVoltage(adcVal, batVoltage);
      showData("OFF", "OFF", "ON", "OFF", adcVal, batVoltage);
      Serial.printf("LED3 ON | ADC: %d | Battery: %.2f V\n", adcVal, batVoltage);
    }
  }

  // ---- LED 4 ON ----
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  digitalWrite(LED3_PIN, LOW);
  delay(SWITCH_DELAY);
  digitalWrite(LED4_PIN, HIGH);
  delay(POST_ON_SETTLE);

  startTime = millis();
  lastMeasure = startTime;
  while (millis() - startTime < LED_ON_TIME) {
    if (millis() - lastMeasure >= MEASURE_INTERVAL) {
      lastMeasure = millis();
      int adcVal;
      float batVoltage;
      readVoltage(adcVal, batVoltage);
      showData("OFF", "OFF", "OFF", "ON", adcVal, batVoltage);
      Serial.printf("LED4 ON | ADC: %d | Battery: %.2f V\n", adcVal, batVoltage);
    }
  }
}