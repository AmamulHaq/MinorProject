#include <Arduino.h>

#define LED_PIN 7

int main(void) {
    init();                 // Initialize Arduino core
    pinMode(LED_PIN, OUTPUT);

    while (1) {
        digitalWrite(LED_PIN, HIGH);
        delay(1000);
        digitalWrite(LED_PIN, LOW);
        delay(1000);
    }
}
