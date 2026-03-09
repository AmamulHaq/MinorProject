#define led 5          // External LED pin
#define onboardLED 2   // ESP32 built-in LED (usually pin 2)
int data;

void setup()
{
  pinMode(led, OUTPUT);
  pinMode(onboardLED, OUTPUT);
  
  Serial.begin(9600);
  
  digitalWrite(led, LOW);
  digitalWrite(onboardLED, LOW);
}

void loop()
{
  while (Serial.available())
  {
    data = Serial.read();

    if (data == '1')
    {
      digitalWrite(led, HIGH);
      digitalWrite(onboardLED, HIGH);
    }
    else if (data == '0')
    {
      digitalWrite(led, LOW);
      digitalWrite(onboardLED, LOW);
    }
  }
}