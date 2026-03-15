#include "Arduino.h"

void setup() {
  Serial.begin(115200);   // Serial to PC
  delay(1000);            // Wait for Serial Monitor
  Serial.println("==================================");
  Serial.println("ESP32-CAM Debug & Handshake Test");
  Serial.println("==================================");
}

void loop() {
  // 1. Send heartbeat to PC
  Serial.println("ESP32-CAM alive");

  // 2. Check if PC sent any command
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n'); // Read command from PC
    cmd.trim();

    Serial.print("Received: ");
    Serial.println(cmd);

    // 3. Double-handshaking: reply back
    if (cmd.length() > 0) {
      Serial.print("ACK: ");       // Acknowledge reception
      Serial.println(cmd);
    }
  }

  // 4. Display free heap memory as debug info
  Serial.print("Free heap: ");
  Serial.println(ESP.getFreeHeap());

  delay(2000);  // Wait 2 seconds before next heartbeat
}