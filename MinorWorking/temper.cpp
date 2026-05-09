#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ========== OLED Settings ==========
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ========== DHT11 Settings ==========
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ========== Pin Definitions (Battery indicators) ==========
#define B1_PIN      14   // B1
#define B2_PIN      27   // B2
#define B3_PIN      26   // B3
#define B4_PIN      25   // B4
#define BATTERY_PIN 32   // GPIO 32 (ADC1) – voltage divider output for total pack voltage

// ========== Timing Constants (ms) ==========
const unsigned long B_ON_TIME        = 5000;   // each B output on for 5 seconds
const unsigned long MEASURE_INTERVAL = 1000;   // take a reading every second
const unsigned long SWITCH_DELAY     = 100;    // all outputs off during switch
const unsigned long POST_ON_SETTLE   = 100;    // settling after output turns on
const unsigned long DHT_INTERVAL     = 2000;   // read DHT11 every 2 seconds

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

// ========== Global DHT variables ==========
float currentTemp = 0.0;
float currentHum = 0.0;
unsigned long lastDHTRead = 0;

// ========== Helper: Read DHT11 (non‑blocking) ==========
void updateDHT() {
  unsigned long now = millis();
  if (now - lastDHTRead >= DHT_INTERVAL) {
    lastDHTRead = now;
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t) && !isnan(h)) {
      currentTemp = t;
      currentHum = h;
      Serial.printf("DHT: Temp = %.1f°C  Humidity = %.1f%%\n", currentTemp, currentHum);
    } else {
      Serial.println("DHT read failed!");
    }
  }
}

// ========== Helper: Read and correct total pack voltage ==========
void readVoltage(int &adcVal, float &correctedVoltage) {
  adcVal = analogRead(BATTERY_PIN);
  float voltage_at_adc = (adcVal * REF_VOLTAGE) / ADC_MAX;
  float raw_battery = voltage_at_adc * (R1 + R2) / R2;
  correctedVoltage = mapVoltage(raw_battery);
}

// ========== Display active battery + temperature ==========
void showActiveBattery(String batteryName, int adcValue, float voltage) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(batteryName);
  display.println(": ON");
  display.print("ADC: ");
  display.println(adcValue);
  display.print("Voltage: ");
  display.print(voltage, 2);
  display.println(" V");
  display.print("Temp: ");
  display.print(currentTemp, 1);
  display.println(" C");
  display.display();
}

// ========== Setup ==========
void setup() {
  Serial.begin(115200);
  pinMode(B1_PIN, OUTPUT);
  pinMode(B2_PIN, OUTPUT);
  pinMode(B3_PIN, OUTPUT);
  pinMode(B4_PIN, OUTPUT);
  digitalWrite(B1_PIN, LOW);
  digitalWrite(B2_PIN, LOW);
  digitalWrite(B3_PIN, LOW);
  digitalWrite(B4_PIN, LOW);

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

  dht.begin();
  // Initial DHT read to fill currentTemp
  updateDHT();
}

// ========== Main Loop – Cycle through B1..B4 outputs ==========
void loop() {
  // ---- B1 ON ----
  digitalWrite(B2_PIN, LOW);
  digitalWrite(B3_PIN, LOW);
  digitalWrite(B4_PIN, LOW);
  delay(SWITCH_DELAY);
  digitalWrite(B1_PIN, HIGH);
  delay(POST_ON_SETTLE);

  unsigned long startTime = millis();
  unsigned long lastMeasure = startTime;
  while (millis() - startTime < B_ON_TIME) {
    if (millis() - lastMeasure >= MEASURE_INTERVAL) {
      lastMeasure = millis();
      int adcVal;
      float packVoltage;
      readVoltage(adcVal, packVoltage);
      updateDHT();                 // Refresh temperature/humidity (non‑blocking)
      showActiveBattery("B1", adcVal, packVoltage);
      Serial.printf("B1 ON | ADC: %d | Pack Voltage: %.2f V\n", adcVal, packVoltage);
    }
  }

  // ---- B2 ON ----
  digitalWrite(B1_PIN, LOW);
  digitalWrite(B3_PIN, LOW);
  digitalWrite(B4_PIN, LOW);
  delay(SWITCH_DELAY);
  digitalWrite(B2_PIN, HIGH);
  delay(POST_ON_SETTLE);

  startTime = millis();
  lastMeasure = startTime;
  while (millis() - startTime < B_ON_TIME) {
    if (millis() - lastMeasure >= MEASURE_INTERVAL) {
      lastMeasure = millis();
      int adcVal;
      float packVoltage;
      readVoltage(adcVal, packVoltage);
      updateDHT();
      showActiveBattery("B2", adcVal, packVoltage);
      Serial.printf("B2 ON | ADC: %d | Pack Voltage: %.2f V\n", adcVal, packVoltage);
    }
  }

  // ---- B3 ON ----
  digitalWrite(B1_PIN, LOW);
  digitalWrite(B2_PIN, LOW);
  digitalWrite(B4_PIN, LOW);
  delay(SWITCH_DELAY);
  digitalWrite(B3_PIN, HIGH);
  delay(POST_ON_SETTLE);

  startTime = millis();
  lastMeasure = startTime;
  while (millis() - startTime < B_ON_TIME) {
    if (millis() - lastMeasure >= MEASURE_INTERVAL) {
      lastMeasure = millis();
      int adcVal;
      float packVoltage;
      readVoltage(adcVal, packVoltage);
      updateDHT();
      showActiveBattery("B3", adcVal, packVoltage);
      Serial.printf("B3 ON | ADC: %d | Pack Voltage: %.2f V\n", adcVal, packVoltage);
    }
  }

  // ---- B4 ON ----
  digitalWrite(B1_PIN, LOW);
  digitalWrite(B2_PIN, LOW);
  digitalWrite(B3_PIN, LOW);
  delay(SWITCH_DELAY);
  digitalWrite(B4_PIN, HIGH);
  delay(POST_ON_SETTLE);

  startTime = millis();
  lastMeasure = startTime;
  while (millis() - startTime < B_ON_TIME) {
    if (millis() - lastMeasure >= MEASURE_INTERVAL) {
      lastMeasure = millis();
      int adcVal;
      float packVoltage;
      readVoltage(adcVal, packVoltage);
      updateDHT();
      showActiveBattery("B4", adcVal, packVoltage);
      Serial.printf("B4 ON | ADC: %d | Pack Voltage: %.2f V\n", adcVal, packVoltage);
    }
  }
}