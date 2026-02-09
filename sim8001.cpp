//Arduino uno
#include <SoftwareSerial.h>

// SoftwareSerial pins
#define SIM800_TX 8   // Arduino TX → SIM800 RX
#define SIM800_RX 7   // Arduino RX → SIM800 TX

SoftwareSerial sim800(SIM800_RX, SIM800_TX);

void sendCommand(const char* cmd, unsigned long delayTime = 2000) {
  sim800.println(cmd);           // Send command to SIM800
  Serial.print("Command: ");
  Serial.println(cmd);

  delay(delayTime);              // Wait for response

  Serial.println("Response:");
  while (sim800.available()) {   // Print all available response
    Serial.write(sim800.read());
  }
  Serial.println("\n----------------------");
}

void setup() {
  Serial.begin(9600);           // Serial Monitor
  sim800.begin(9600);           // SIM800 baud rate

  Serial.println("SIM800L Automatic AT Command Test");
  Serial.println("------------------------------------");
  delay(3000);                  // Give SIM800 time to boot

  // Test communication
  sendCommand("AT");

  // Check SIM card
  sendCommand("AT+CPIN?");

  //  Check signal strength
  sendCommand("AT+CSQ");

  //  Check network registration
  sendCommand("AT+CREG?");

  //  Set SMS text mode
  sendCommand("AT+CMGF=1");

  //  Send SMS (replace number)
  sendCommand("AT+CMGS=\"+9779868049818\""); // Enter recipient
  delay(1000);
  sim800.print("Hello from Amamul + SIM800L");
  sim800.write(26); // CTRL+Z to send SMS
  Serial.println("SMS Sent Command Executed\n----------------------");
}

void loop() {
  // Nothing needed here
}

