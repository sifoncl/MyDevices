#include <Arduino.h>
#include <Wire.h>

#include "AppConfig.h"
#include "ButtonInputController.h"
#include "ClimateSensor.h"
#include "HomeAssistantBridge.h"
#include "MqttSettings.h"
#include "NetworkManager.h"
#include "PressureSensor.h"
#include "RelayController.h"
#include "WebInterface.h"

NetworkManager network(
    Config::WIFI_DEFAULT_SSID,
    Config::WIFI_DEFAULT_PASSWORD,
    Config::WIFI_SETUP_AP_SSID,
    Config::WIFI_SETUP_AP_PASSWORD,
    Config::WIFI_HOSTNAME,
    Config::WIFI_CONNECT_TIMEOUT_MS,
    Config::WIFI_RECONNECT_INTERVAL_MS,
    Config::wifiSetupApIp());

MqttSettings mqttSettings(
    Config::MQTT_SERVER,
    Config::MQTT_PORT,
    Config::MQTT_USER,
    Config::MQTT_PASSWORD);

RelayController relays(
    Config::RELAY_PINS,
    Config::RELAY_COUNT,
    Config::RELAY_ACTIVE_LEVEL);

ButtonInputController buttons(
    Config::BUTTON_PINS,
    Config::RELAY_COUNT,
    Config::BUTTON_ACTIVE_LEVEL,
    Config::BUTTON_INPUT_MODE,
    Config::BUTTON_DEBOUNCE_MS,
    relays);

ClimateSensor climate(
    Config::DHT_PIN,
    Config::SENSOR_READ_INTERVAL_MS,
    Config::SENSOR_UNAVAILABLE_DELAY_MS);

PressureSensor pressure(
    Wire,
    Config::PRESSURE_SENSOR_ADDRESS,
    Config::PRESSURE_SENSOR_ENABLED,
    Config::SENSOR_READ_INTERVAL_MS,
    Config::SENSOR_UNAVAILABLE_DELAY_MS);

HomeAssistantBridge homeAssistant(
    relays,
    climate,
    pressure,
    network,
    Config::DEVICE_ID,
    Config::HA_PUBLISH_INTERVAL_MS);

WebInterface web(
    network,
    relays,
    climate,
    pressure,
    homeAssistant,
    mqttSettings);

namespace
{
void scanI2cBus()
{
  uint8_t devicesFound = 0;
  Serial.print("I2C scan on SDA=");
  Serial.print(Config::I2C_SDA_PIN);
  Serial.print(", SCL=");
  Serial.println(Config::I2C_SCL_PIN);

  for (uint8_t address = 1; address < 127; address++)
  {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0)
    {
      Serial.print("I2C device found at 0x");
      if (address < 16)
      {
        Serial.print("0");
      }
      Serial.println(address, HEX);
      devicesFound++;
    }
  }

  if (devicesFound == 0)
  {
    Serial.println("I2C scan: no devices found");
  }
}
}

void setup()
{
  Serial.begin(Config::SERIAL_BAUD);
  delay(300);

  Serial.println();
  Serial.print(Config::DEVICE_NAME);
  Serial.print(" v");
  Serial.println(Config::SOFTWARE_VERSION);

  mqttSettings.begin();
  network.begin();

  Wire.begin(Config::I2C_SDA_PIN, Config::I2C_SCL_PIN);
  Wire.setTimeOut(50);
  scanI2cBus();

  relays.begin();
  buttons.begin();
  climate.begin();
  pressure.begin();

  homeAssistant.begin(
      mqttSettings.server().c_str(),
      mqttSettings.port(),
      mqttSettings.user().isEmpty() ? nullptr : mqttSettings.user().c_str(),
      mqttSettings.password().isEmpty() ? nullptr : mqttSettings.password().c_str());

  web.begin();
}

void loop()
{
  network.loop();
  web.loop();
  buttons.loop();
  climate.loop();
  pressure.loop();
  homeAssistant.loop();
}
