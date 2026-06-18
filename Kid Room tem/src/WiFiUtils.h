#ifndef WIFI_UTILS_H
#define WIFI_UTILS_H

#include <Arduino.h>
#include <WiFi.h>

class WiFiUtils
{
public:
  static bool connect(const char *ssid, const char *password, uint8_t maxAttempts = 20, unsigned long retryDelayMs = 500, bool restartOnFail = true);
  static bool setupWiFi(const char *ssid, const char *password, uint8_t maxAttempts = 20, unsigned long retryDelayMs = 500, bool restartOnFail = true);
  static void beginAutoReconnect(const char *ssid, const char *password, unsigned long retryIntervalMs = 10000UL);
  static void loop();
  static bool isConnected();
};

#endif
