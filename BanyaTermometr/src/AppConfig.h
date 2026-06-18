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
constexpr char WIFI_SETUP_AP_SSID[] = "BanyaTermometr-Setup";
constexpr char WIFI_SETUP_AP_PASSWORD[] = CONFIG_WIFI_SETUP_AP_PASSWORD;
constexpr char WIFI_HOSTNAME[] = "banya-thermometer";

constexpr char MQTT_SERVER[] = CONFIG_MQTT_SERVER;
constexpr uint16_t MQTT_PORT = 1883;
constexpr char MQTT_USER[] = CONFIG_MQTT_USER;
constexpr char MQTT_PASSWORD[] = CONFIG_MQTT_PASSWORD;
constexpr char MIRROR_MQTT_TEMPERATURE_TOPIC[] = "banya_thermometer/sensor/temperature";
constexpr char MIRROR_MQTT_HUMIDITY_TOPIC[] = "banya_thermometer/sensor/humidity";
constexpr char MIRROR_MQTT_VALID_TOPIC[] = "banya_thermometer/sensor/valid";

constexpr char DEVICE_NAME[] = "Banya Thermometer";
constexpr char DEVICE_ID[] = "banya_thermometer";
constexpr char SOFTWARE_VERSION[] = "4.0.0";

constexpr uint32_t SERIAL_BAUD = 115200;

// ESP32-S3 Super Mini, matching the pin strategy used by Kid Room tem.
constexpr uint8_t I2C_SDA_PIN = 5;
constexpr uint8_t I2C_SCL_PIN = 3;
constexpr uint8_t MATRIX_DATA_PIN = 13;
constexpr uint8_t MATRIX_CLOCK_PIN = 11;
constexpr uint8_t MATRIX_CS_PIN = 12;
constexpr uint8_t MATRIX_SEGMENTS = 4;
constexpr uint8_t MATRIX_MIN_INTENSITY = 0;
constexpr uint8_t MATRIX_MAX_INTENSITY = 15;
constexpr uint8_t MATRIX_INTENSITY = 15;
constexpr bool MATRIX_REVERSE_SEGMENT_ORDER = false;
constexpr bool MATRIX_MIRROR_COLUMNS = true;
constexpr bool MATRIX_MIRROR_ROWS = true;

constexpr uint8_t SHT85_ADDRESS = 0x44;
constexpr uint32_t SENSOR_READ_INTERVAL_MS = 5000UL;
constexpr uint32_t SENSOR_RETRY_INTERVAL_MS = 30000UL;
constexpr uint32_t DISPLAY_RENDER_INTERVAL_MS = 250UL;
constexpr uint32_t DISPLAY_MODE_INTERVAL_MS = 10000UL;
constexpr uint32_t HA_PUBLISH_INTERVAL_MS = 5000UL;
constexpr uint32_t ESPNOW_PUBLISH_INTERVAL_MS = 5000UL;

constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 12000UL;
constexpr uint32_t WIFI_RECONNECT_INTERVAL_MS = 10000UL;

inline IPAddress wifiSetupApIp()
{
  return IPAddress(192, 168, 4, 1);
}
}

#endif
