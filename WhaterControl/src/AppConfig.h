#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <Arduino.h>
#include <IPAddress.h>

#if __has_include("Secrets.h")
#include "Secrets.h"
#endif

#ifndef CONFIG_WIFI_DEFAULT_SSID
#define CONFIG_WIFI_DEFAULT_SSID "CHANGE_ME"
#endif
#ifndef CONFIG_WIFI_DEFAULT_PASSWORD
#define CONFIG_WIFI_DEFAULT_PASSWORD "CHANGE_ME"
#endif
#ifndef CONFIG_WIFI_SETUP_AP_PASSWORD
#define CONFIG_WIFI_SETUP_AP_PASSWORD "CHANGE_ME"
#endif
#ifndef CONFIG_MQTT_SERVER
#define CONFIG_MQTT_SERVER "192.168.1.100"
#endif
#ifndef CONFIG_MQTT_USER
#define CONFIG_MQTT_USER "CHANGE_ME"
#endif
#ifndef CONFIG_MQTT_PASSWORD
#define CONFIG_MQTT_PASSWORD "CHANGE_ME"
#endif

namespace Config
{
constexpr char WIFI_DEFAULT_SSID[] = CONFIG_WIFI_DEFAULT_SSID;
constexpr char WIFI_DEFAULT_PASSWORD[] = CONFIG_WIFI_DEFAULT_PASSWORD;
constexpr char WIFI_SETUP_AP_SSID[] = "WhaterControl-Setup";
constexpr char WIFI_SETUP_AP_PASSWORD[] = CONFIG_WIFI_SETUP_AP_PASSWORD;
constexpr char WIFI_HOSTNAME[] = "whater-control";

constexpr char MQTT_SERVER[] = CONFIG_MQTT_SERVER;
constexpr uint16_t MQTT_PORT = 1883;
constexpr char MQTT_USER[] = CONFIG_MQTT_USER;
constexpr char MQTT_PASSWORD[] = CONFIG_MQTT_PASSWORD;

constexpr char DEVICE_NAME[] = "Water Control";
constexpr char DEVICE_ID[] = "custom_4";
constexpr char SOFTWARE_VERSION[] = "2.0.0";

constexpr uint32_t SERIAL_BAUD = 115200;

constexpr uint8_t RELAY_COUNT = 4;
constexpr uint8_t RELAY_PINS[RELAY_COUNT] = {13, 12, 11, 10};
constexpr uint8_t BUTTON_PINS[RELAY_COUNT] = {1, 2, 3, 4};
constexpr uint8_t RELAY_ACTIVE_LEVEL = LOW;
constexpr uint8_t BUTTON_ACTIVE_LEVEL = HIGH;
constexpr uint8_t BUTTON_INPUT_MODE = INPUT_PULLUP;
constexpr uint32_t BUTTON_DEBOUNCE_MS = 50UL;

constexpr uint8_t DHT_PIN = 9;
constexpr uint32_t SENSOR_READ_INTERVAL_MS = 4200UL;
constexpr uint32_t SENSOR_UNAVAILABLE_DELAY_MS = 10000UL;

constexpr uint8_t I2C_SDA_PIN = 5;
constexpr uint8_t I2C_SCL_PIN = 3;
constexpr uint8_t PRESSURE_SENSOR_ADDRESS = 0x76;
constexpr bool PRESSURE_SENSOR_ENABLED = true;

constexpr uint32_t HA_PUBLISH_INTERVAL_MS = 2000UL;
constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 12000UL;
constexpr uint32_t WIFI_RECONNECT_INTERVAL_MS = 10000UL;

inline IPAddress wifiSetupApIp()
{
  return IPAddress(192, 168, 4, 1);
}
}

#endif
