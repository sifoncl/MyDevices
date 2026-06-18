#include <Arduino.h>

#include "AppConfig.h"
#include "DisplaySettings.h"
#include "HomeAssistantBridge.h"
#include "MatrixDisplay.h"
#include "MqttSettings.h"
#include "NetworkManager.h"
#include "RemoteClimateSource.h"
#include "TemperatureBoostSettings.h"
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

TemperatureBoostSettings boostSettings(
    true,
    Config::TEMPERATURE_BOOST_DEFAULT_FACTOR,
    Config::TEMPERATURE_BOOST_MIN_FACTOR,
    Config::TEMPERATURE_BOOST_MAX_FACTOR,
    Config::TEMPERATURE_BOOST_BASELINE);

RemoteClimateSource climateSource(
    Config::SOURCE_MQTT_CLIENT_ID,
    Config::HA_TEMPERATURE_TOPIC,
    Config::HA_HUMIDITY_TOPIC,
    Config::MIRROR_MQTT_TEMPERATURE_TOPIC,
    Config::MIRROR_MQTT_HUMIDITY_TOPIC,
    Config::MIRROR_MQTT_VALID_TOPIC,
    Config::SOURCE_MQTT_RECONNECT_INTERVAL_MS,
    Config::SOURCE_SAMPLE_TIMEOUT_MS);

HomeAssistantBridge homeAssistant(
    climateSource,
    boostSettings,
    display,
    displaySettings,
    network,
    Config::DEVICE_ID,
    Config::HA_PUBLISH_INTERVAL_MS);

WebInterface web(
    network,
    climateSource,
    homeAssistant,
    mqttSettings,
    display,
    displaySettings,
    boostSettings);

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
  boostSettings.begin();

  network.begin();
  climateSource.begin(
      mqttSettings.server().c_str(),
      mqttSettings.port(),
      mqttSettings.user().isEmpty() ? nullptr : mqttSettings.user().c_str(),
      mqttSettings.password().isEmpty() ? nullptr : mqttSettings.password().c_str());

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
  climateSource.loop();
  display.loop(climateSource.current(boostSettings));
  homeAssistant.loop();
}
