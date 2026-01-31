#include <Arduino.h>
// to use the multiplexer CD4051
// ================= CD4051 SELECTION PINS =================
#define SEL_A 2   // CD4051 pin 11
#define SEL_B 3   // CD4051 pin 10
// CD4051 pin 9 (C) is tied to GND

// ================= PWM INPUTS TO CD4051 ==================
#define PIN_12_PWM 6    // CD4051 pin 12  (Dim)
#define PIN_13_PWM 9    // CD4051 pin 13  (Little bright)
#define PIN_14_PWM 10   // CD4051 pin 14  (Brighter)
#define PIN_15_PWM 11   // CD4051 pin 15  (Very bright)

// ================= BRIGHTNESS VALUES =====================
#define PWM_12 40
#define PWM_13 100
#define PWM_14 180
#define PWM_15 255

// ================= FUNCTION ===============================
void selectChannel(uint8_t channel)
{
    digitalWrite(SEL_A, channel & 0x01);          // A = bit0
    digitalWrite(SEL_B, (channel >> 1) & 0x01);   // B = bit1
}

// ================= SETUP ===============================
void setup()
{
    Serial.begin(9600);

    pinMode(SEL_A, OUTPUT);
    pinMode(SEL_B, OUTPUT);

    pinMode(PIN_12_PWM, OUTPUT);
    pinMode(PIN_13_PWM, OUTPUT);
    pinMode(PIN_14_PWM, OUTPUT);
    pinMode(PIN_15_PWM, OUTPUT);

    // Set fixed PWM brightness levels
    analogWrite(PIN_12_PWM, PWM_12);
    analogWrite(PIN_13_PWM, PWM_13);
    analogWrite(PIN_14_PWM, PWM_14);
    analogWrite(PIN_15_PWM, PWM_15);

    Serial.println("CD4051 PWM Multiplexing Test Started");
    Serial.println("----------------------------------");
}

// ================= LOOP ===============================
void loop()
{
    selectChannel(3);  // CH3 → CD4051 pin 12
    Serial.println("Selected CH3 (Pin 12) | PWM = 40 (Dim)");
    delay(5000);

    selectChannel(0);  // CH0 → CD4051 pin 13
    Serial.println("Selected CH0 (Pin 13) | PWM = 100 (Little Bright)");
    delay(5000);

    selectChannel(1);  // CH1 → CD4051 pin 14
    Serial.println("Selected CH1 (Pin 14) | PWM = 180 (Brighter)");
    delay(5000);

    selectChannel(2);  // CH2 → CD4051 pin 15
    Serial.println("Selected CH2 (Pin 15) | PWM = 255 (Very Bright)");
    delay(5000);
}
