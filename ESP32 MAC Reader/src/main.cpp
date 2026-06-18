#include <Arduino.h>
#include <WiFi.h>

void printAddresses()
{
  Serial.println();
  Serial.println("=== ESP32 MAC addresses ===");
  Serial.print("STA MAC (use for ESP-NOW): ");
  Serial.println(WiFi.macAddress());
  Serial.print("AP MAC:                    ");
  Serial.println(WiFi.softAPmacAddress());
  Serial.println();
  Serial.println("Use the STA MAC in CONTROLLER_MAC, UPPER_SENSOR_MAC or LOWER_SENSOR_MAC.");
  Serial.println("===========================");
}

void setup()
{
  Serial.begin(115200);
  delay(1200);

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("ESP32-MAC-Reader");
  delay(100);
  printAddresses();
}

void loop()
{
  static unsigned long lastPrintAt = 0;
  if ((millis() - lastPrintAt) >= 5000UL)
  {
    lastPrintAt = millis();
    printAddresses();
  }
}
