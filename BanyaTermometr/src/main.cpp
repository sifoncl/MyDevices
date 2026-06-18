#include <Arduino.h>
#include <Wire.h>

#include "AppConfig.h"
#include "ClimateSensor.h"
#include "DisplaySettings.h"
#include "EspNowClimateBroadcaster.h"
#include "HomeAssistantBridge.h"
#include "MatrixDisplay.h"
#include "MqttSettings.h"
#include "NetworkManager.h"
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

ClimateSensor climate(
    Wire,
    Config::SHT85_ADDRESS,
    Config::SENSOR_READ_INTERVAL_MS,
    Config::SENSOR_RETRY_INTERVAL_MS);

MatrixDisplay display(
    Config::MATRIX_DATA_PIN,
    Config::MATRIX_CLOCK_PIN,
    Config::MATRIX_CS_PIN,
    Config::MATRIX_SEGMENTS,
    Config::MATRIX_INTENSITY,
    Config::DISPLAY_RENDER_INTERVAL_MS,
    Config::DISPLAY_MODE_INTERVAL_MS);

MqttSettings mqttSettings(
    Config::MQTT_SERVER,
    Config::MQTT_PORT,
    Config::MQTT_USER,
    Config::MQTT_PASSWORD);

DisplaySettings displaySettings(
    Config::MATRIX_INTENSITY,
    Config::MATRIX_MIN_INTENSITY,
    Config::MATRIX_MAX_INTENSITY);

EspNowClimateBroadcaster espNowBroadcaster(Config::ESPNOW_PUBLISH_INTERVAL_MS);

HomeAssistantBridge homeAssistant(
    climate,
    display,
    displaySettings,
    network,
    Config::DEVICE_ID,
    Config::HA_PUBLISH_INTERVAL_MS);

WebInterface web(
    network,
    climate,
    homeAssistant,
    mqttSettings,
    display,
    displaySettings);

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
    Serial.println("I2C scan: no devices found; check 3.3V, GND, SDA and SCL");
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
  displaySettings.begin();

  network.begin();
  espNowBroadcaster.begin();

  Wire.begin(Config::I2C_SDA_PIN, Config::I2C_SCL_PIN);
  Wire.setTimeOut(50);
  scanI2cBus();
  climate.begin();
  display.setEnabled(displaySettings.enabled());
  display.begin();
  display.setIntensity(static_cast<uint8_t>(displaySettings.brightness()));
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
  climate.loop();
  espNowBroadcaster.loop(climate);
  display.loop(climate);
  homeAssistant.loop();
}
