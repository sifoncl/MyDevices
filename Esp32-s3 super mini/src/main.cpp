#include <Arduino.h>


void setup() {
    Serial.begin(115200);
    delay(10000);
    Serial.println("Hello, ESP32111111!");  
  pinMode(BUILTIN_LED, OUTPUT);
}

void loop() {
      Serial.println("Hello, ESP32111111!");  
digitalWrite(BUILTIN_LED, true);
delay(1000);
digitalWrite(BUILTIN_LED, false);
delay(1000);
}
