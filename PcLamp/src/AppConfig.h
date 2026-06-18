#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <Arduino.h>
#include <IPAddress.h>

#if __has_include("Secrets.h")
#include "Secrets.h"
#endif

#ifndef WIFI_SSID
#define WIFI_SSID "CHANGE_ME"
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "CHANGE_ME"
#endif
#ifndef WIFI_SETUP_AP_SSID
#define WIFI_SETUP_AP_SSID "PcLamp-Setup"
#endif
#ifndef WIFI_SETUP_AP_PASSWORD
#define WIFI_SETUP_AP_PASSWORD "CHANGE_ME"
#endif
#ifndef WIFI_HOSTNAME
#define WIFI_HOSTNAME "pc-lamp"
#endif

#ifndef MQTT_SERVER
#define MQTT_SERVER "192.168.1.100"
#endif
#ifndef MQTT_PORT
#define MQTT_PORT 1883
#endif
#ifndef MQTT_USER
#define MQTT_USER "CHANGE_ME"
#endif
#ifndef MQTT_PASSWORD
#define MQTT_PASSWORD "CHANGE_ME"
#endif

namespace Config
{
constexpr char WIFI_DEFAULT_SSID[] = WIFI_SSID;
constexpr char WIFI_DEFAULT_PASSWORD[] = WIFI_PASSWORD;
constexpr char WIFI_SETUP_AP_SSID_VALUE[] = WIFI_SETUP_AP_SSID;
constexpr char WIFI_SETUP_AP_PASSWORD_VALUE[] = WIFI_SETUP_AP_PASSWORD;
constexpr char WIFI_HOSTNAME_VALUE[] = WIFI_HOSTNAME;

constexpr char MQTT_SERVER_VALUE[] = MQTT_SERVER;
constexpr uint16_t MQTT_PORT_VALUE = MQTT_PORT;
constexpr char MQTT_USER_VALUE[] = MQTT_USER;
constexpr char MQTT_PASSWORD_VALUE[] = MQTT_PASSWORD;

constexpr char DEVICE_NAME[] = "PC Lamp and Power Controller";
constexpr char DEVICE_ID[] = "pc_lamp_controller";
constexpr char SOFTWARE_VERSION[] = "2.0.0";
constexpr uint32_t SERIAL_BAUD = 115200UL;

constexpr uint8_t PC_STATUS_PIN = 12;
constexpr uint8_t PC_POWER_BUTTON_PIN = 14;
constexpr uint8_t PC_STATUS_INPUT_MODE = INPUT_PULLUP;
constexpr bool PC_STATUS_ACTIVE_HIGH = true;
constexpr bool PC_POWER_BUTTON_ACTIVE_HIGH = true;
constexpr uint32_t PC_STATUS_DEBOUNCE_MS = 800UL;
constexpr uint32_t PC_POWER_BUTTON_PULSE_MS = 250UL;
constexpr uint32_t PC_START_RETRY_MS = 10000UL;
constexpr uint32_t PC_SHUTDOWN_RETRY_MS = 12000UL;
constexpr uint8_t PC_MAX_START_ATTEMPTS = 2;
constexpr uint8_t PC_MAX_SHUTDOWN_ATTEMPTS = 2;

constexpr uint8_t RED_PIN = 18;
constexpr uint8_t GREEN_PIN = 19;
constexpr uint8_t BLUE_PIN = 5;
constexpr uint8_t WHITE_PIN = 17;
constexpr uint8_t WARM_WHITE_PIN = 21;
constexpr uint16_t MAX_COLOR_TEMP_MIRED = 454;
constexpr uint16_t MIN_COLOR_TEMP_MIRED = 153;
constexpr uint8_t DEFAULT_LIGHT_BRIGHTNESS = 96;
constexpr uint16_t DEFAULT_LIGHT_COLOR_TEMP_MIRED = 320;

constexpr uint8_t DHT_DATA_PIN = 25;
constexpr uint8_t DHT_TYPE_VALUE = 22;
constexpr uint32_t SENSOR_READ_INTERVAL_MS = 4200UL;
constexpr uint32_t SENSOR_INVALID_TIMEOUT_MS = 10000UL;

constexpr uint8_t TOUCH_BUTTON_PIN = 32;
constexpr uint16_t TOUCH_THRESHOLD = 10;
constexpr uint32_t TOUCH_DEBOUNCE_MS = 100UL;

constexpr uint32_t HA_PUBLISH_INTERVAL_MS = 2000UL;
constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 12000UL;
constexpr uint32_t WIFI_RECONNECT_INTERVAL_MS = 10000UL;

inline IPAddress wifiSetupApIp()
{
  return IPAddress(192, 168, 4, 1);
}
}

#endif
