#ifndef HOME_ASSISTANT_BRIDGE_H
#define HOME_ASSISTANT_BRIDGE_H

#include <ArduinoHA.h>
#include <WiFi.h>

#include "DisplaySettings.h"
#include "MatrixDisplay.h"
#include "NetworkManager.h"
#include "RemoteClimateSource.h"
#include "TemperatureBoostSettings.h"

class HomeAssistantBridge
{
public:
  HomeAssistantBridge(RemoteClimateSource &source,
                      TemperatureBoostSettings &boost,
                      MatrixDisplay &display,
                      DisplaySettings &displaySettings,
                      NetworkManager &network,
                      const char *deviceId,
                      uint32_t publishIntervalMs);

  void begin(const char *mqttServer,
             uint16_t mqttPort,
             const char *mqttUser,
             const char *mqttPassword);
  bool reconfigure(const char *mqttServer,
                   uint16_t mqttPort,
                   const char *mqttUser,
                   const char *mqttPassword);
  void loop();
  bool connected() const;

private:
  static HomeAssistantBridge *_instance;

  RemoteClimateSource &_source;
  TemperatureBoostSettings &_boost;
  MatrixDisplay &_display;
  DisplaySettings &_displaySettings;
  NetworkManager &_network;
  WiFiClient _client;
  HADevice _device;
  HAMqtt _mqtt;
  HASensorNumber _temperature;
  HASensorNumber _rawTemperature;
  HASensorNumber _humidity;
  HASensor _sourceName;
  HANumber _displayBrightness;
  HASwitch _displayEnabled;
  HANumber _boostFactor;
  HASwitch _boostEnabled;
  uint32_t _publishIntervalMs;
  uint32_t _lastPublishAt = 0;
  uint32_t _lastRevision = UINT32_MAX;
  char _configurationUrl[64] = "";

  void publish(bool force);
  static void onMqttConnected();
  static void onBrightnessCommand(HANumeric number, HANumber *sender);
  static void onDisplayEnabledCommand(bool state, HASwitch *sender);
  static void onBoostFactorCommand(HANumeric number, HANumber *sender);
  static void onBoostEnabledCommand(bool state, HASwitch *sender);
};

#endif
