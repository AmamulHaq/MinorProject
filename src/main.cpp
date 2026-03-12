#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ESP32 pins
#define LED1_PIN 18
#define LED2_PIN 19
#define ANALOG_PIN 34

const float referenceVoltage = 3.3;

void setup()
{

  Serial.begin(115200);

  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);

  // Start I2C for OLED
  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println("OLED not found");
    while (true)
      ;
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
}

void showData(String led1, String led2, int adcValue, float voltage)
{

  display.clearDisplay();

  display.setCursor(0, 0);
  display.print("LED1: ");
  display.println(led1);

  display.print("LED2: ");
  display.println(led2);

  display.print("ADC: ");
  display.println(adcValue);

  display.print("Voltage: ");
  display.print(voltage, 2);
  display.println(" V");

  display.display();
}

void loop()
{

  // LED1 ON
  digitalWrite(LED1_PIN, HIGH);
  digitalWrite(LED2_PIN, LOW);

  unsigned long startTime = millis();

  while (millis() - startTime < 5000)
  {

    int adcValue = analogRead(ANALOG_PIN);
    float voltage = (adcValue * referenceVoltage) / 4095.0;

    Serial.print("LED1 ON | ADC: ");
    Serial.print(adcValue);
    Serial.print(" | Voltage: ");
    Serial.println(voltage);

    showData("ON", "OFF", adcValue, voltage);

    delay(1000);
  }

  // LED2 ON
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, HIGH);

  startTime = millis();

  while (millis() - startTime < 5000)
  {

    int adcValue = analogRead(ANALOG_PIN);
    float voltage = (adcValue * referenceVoltage) / 4095.0;

    Serial.print("LED2 ON | ADC: ");
    Serial.print(adcValue);
    Serial.print(" | Voltage: ");
    Serial.println(voltage);

    showData("OFF", "ON", adcValue, voltage);

    delay(1000);
  }
}