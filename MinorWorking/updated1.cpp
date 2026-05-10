#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>

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

// ========== Alert Output Pins ==========
#define ALERT_PIN1  2
#define ALERT_PIN2  19

// ========== WiFi Credentials ==========
const char* ssid     = "God Father";        // change to your WiFi name
const char* password = "#godfather786";    // change to your WiFi password

// ========== Flask Server URL ==========
const char* serverUrl = "http://192.168.31.8:5000/insert";  // change IP to your server

// ========== Measurement Constants ==========
#define NUM_BATTERIES        4
#define SAMPLES_PER_MEASURE  4          // number of readings averaged per battery
#define SAMPLE_DELAY_MS      500        // delay between samples (ms)
const float MISSING_BATTERY_VOLTAGE = 0.68;  // voltage when no battery connected
const float MISSING_TOLERANCE       = 0.05;
const unsigned long SWITCH_DELAY        = 100;
const unsigned long POST_ON_SETTLE      = 100;
const unsigned long DHT_INTERVAL        = 2000;
const unsigned long DISPLAY_DURATION_MS = 5000;   // how long to show summary

// ========== Voltage Divider & ADC ==========
const float REF_VOLTAGE   = 3.3;
const float ADC_MAX       = 4095.0;
const float R1            = 30000.0;
const float R2            = 7500.0;

// ========== Calibration Mapping ==========
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

// ========== Read single corrected voltage ==========
float readSingleVoltage() {
  int adcVal = analogRead(BATTERY_PIN);
  float voltage_at_adc = (adcVal * REF_VOLTAGE) / ADC_MAX;
  float raw_battery = voltage_at_adc * (R1 + R2) / R2;
  return mapVoltage(raw_battery);
}

// ========== Update alert pins ==========
void updateAlertPins() {
  bool hot = (currentTemp > 35.0);
  digitalWrite(ALERT_PIN1, hot ? HIGH : LOW);
  digitalWrite(ALERT_PIN2, hot ? HIGH : LOW);
}

// ========== Display during measurement (Bx: ON + live voltage) ==========
void showMeasurementScreen(String batteryName, float liveVoltage) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(batteryName);
  display.println(": ON");
  display.print("Total Battery: ");
  display.print(liveVoltage, 2);
  display.println(" V");
  display.print("Temp: ");
  display.print(currentTemp, 1);
  display.println(" C");
  if (currentTemp > 40.0) display.println("Alert: High Temp");
  display.display();
}

// ========== Measure cumulative voltage for a given battery indicator ==========
float measureCumulativeVoltage(int batteryPin, const char* name) {
  // Turn off all pins, then turn on the selected one
  digitalWrite(B1_PIN, LOW);
  digitalWrite(B2_PIN, LOW);
  digitalWrite(B3_PIN, LOW);
  digitalWrite(B4_PIN, LOW);
  delay(SWITCH_DELAY);
  digitalWrite(batteryPin, HIGH);
  delay(POST_ON_SETTLE);

  float sum = 0.0;
  for (int i = 0; i < SAMPLES_PER_MEASURE; i++) {
    float v = readSingleVoltage();
    sum += v;
    Serial.printf("%s sample %d: %.3f V\n", name, i+1, v);
    // Show live reading on OLED
    showMeasurementScreen(String(name), v);
    delay(SAMPLE_DELAY_MS);
    updateDHT();          // keep temperature fresh
    updateAlertPins();
  }
  float average = sum / SAMPLES_PER_MEASURE;
  Serial.printf("%s cumulative average = %.3f V\n", name, average);

  // If the average is near the "missing battery" voltage, treat as 0
  if (fabs(average - MISSING_BATTERY_VOLTAGE) <= MISSING_TOLERANCE) {
    Serial.printf("  -> Battery missing! Setting cumulative to 0.0 V\n");
    return 0.0;
  }
  return average;
}

// ========== Send data to Flask server ==========
void sendToServer(float b1, float b2, float b3, float b4, float total, float temp) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(serverUrl) + "?B1=" + String(b1, 3) +
                 "&B2=" + String(b2, 3) +
                 "&B3=" + String(b3, 3) +
                 "&B4=" + String(b4, 3) +
                 "&TotalVoltage=" + String(total, 3) +
                 "&Temp=" + String(temp, 1);
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0) {
      Serial.printf("HTTP GET returned %d\n", httpCode);
    } else {
      Serial.printf("HTTP GET failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
}

// ========== Display final summary (individual voltages + net pack) ==========
void displaySummary(float individual[], float netPack) {
  display.clearDisplay();
  display.setCursor(0, 0);
  for (int i = 0; i < NUM_BATTERIES; i++) {
    display.print("B"); display.print(i+1); display.print(": ");
    display.print(individual[i], 3); display.println(" V");
  }
  display.print("Net Pack: ");
  display.print(netPack, 3);
  display.println(" V");
  display.print("Temp: ");
  display.print(currentTemp, 1);
  display.println(" C");
  if (currentTemp > 35.0) display.println("Alert: High Temp");
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

  pinMode(ALERT_PIN1, OUTPUT);
  pinMode(ALERT_PIN2, OUTPUT);
  digitalWrite(ALERT_PIN1, LOW);
  digitalWrite(ALERT_PIN2, LOW);

  analogSetAttenuation(ADC_11db);
  analogReadResolution(12);

  Wire.begin(21, 22);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found");
    while (true);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  dht.begin();
  updateDHT();

  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected. IP address: " + WiFi.localIP().toString());
}

// ========== Main Loop ==========
void loop() {
  float cumulative[NUM_BATTERIES];
  float individual[NUM_BATTERIES];

  // ----- Measure cumulative voltages -----
  cumulative[0] = measureCumulativeVoltage(B1_PIN, "B1");
  cumulative[1] = measureCumulativeVoltage(B2_PIN, "B1+B2");
  cumulative[2] = measureCumulativeVoltage(B3_PIN, "B1+B2+B3");
  cumulative[3] = measureCumulativeVoltage(B4_PIN, "B1+B2+B3+B4");

  // ----- Compute individual voltages by subtraction -----
  individual[0] = cumulative[0];
  for (int i = 1; i < NUM_BATTERIES; i++) {
    individual[i] = cumulative[i] - cumulative[i-1];
    if (individual[i] < 0) individual[i] = 0.0;
  }

  float netPack = 0;
  for (int i = 0; i < NUM_BATTERIES; i++) netPack += individual[i];

  // ----- Send data to database via Flask -----
  sendToServer(individual[0], individual[1], individual[2], individual[3], netPack, currentTemp);

  // ----- Print to Serial -----
  Serial.println("\n--- Individual Battery Voltages ---");
  for (int i = 0; i < NUM_BATTERIES; i++) {
    Serial.printf("B%d = %.3f V\n", i+1, individual[i]);
  }
  Serial.printf("Net Pack = %.3f V\n", netPack);
  Serial.println();

  // ----- Show summary on OLED for 5 seconds -----
  unsigned long displayStart = millis();
  while (millis() - displayStart < DISPLAY_DURATION_MS) {
    updateDHT();
    updateAlertPins();
    displaySummary(individual, netPack);
    delay(500);
  }
}