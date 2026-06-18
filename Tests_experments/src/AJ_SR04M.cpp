#include<Arduino.h>

#define TRIG_PIN 19  // Pin connected to Trig
#define ECHO_PIN 18  // Pin connected to Echo

void setup() {
  Serial.begin(115200);       // Initialize Serial Monitor
  pinMode(TRIG_PIN, OUTPUT); // Set Trig as output
  pinMode(ECHO_PIN, INPUT);  // Set Echo as input
  Serial.println("AJ-SR04M in HR-04 Trigger Mode Initialized");
}

void loop() {
  // Trigger the ultrasonic pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Measure the duration of the pulse
  long duration = pulseIn(ECHO_PIN, HIGH);

  // Calculate the distance in cm
  float distance = (duration * 0.034) / 2;

  // Display the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  Serial.print("Pulse duration: ");
  Serial.println(duration);


  delay(1000); // Wait for half a second before the next measurement
}