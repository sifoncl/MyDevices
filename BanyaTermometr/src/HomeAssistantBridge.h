#ifndef HOME_ASSISTANT_BRIDGE_H
#define HOME_ASSISTANT_BRIDGE_H

#include <ArduinoHA.h>
#include <WiFi.h>

#include "ClimateSensor.h"
#include "DisplaySettings.h"
#include "MatrixDisplay.h"
#include "NetworkManager.h"

class HomeAssistantBridge
{
public:
  HomeAssistantBridge(ClimateSensor &climate,
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

  ClimateSensor &_climate;
  MatrixDisplay &_display;
  DisplaySettings &_displaySettings;
  NetworkManager &_network;
  WiFiClient _client;
  HADevice _device;
  HAMqtt _mqtt;
  HASensorNumber _temperature;
  HASensorNumber _humidity;
  HANumber _displayBrightness;
  HASwitch _displayEnabled;
  uint32_t _publishIntervalMs;
  uint32_t _lastPublishAt = 0;
  uint32_t _lastClimateRevision = UINT32_MAX;
  char _configurationUrl[64] = "";

  void publish(bool force);
  static void onMqttConnected();
  static void onBrightnessCommand(HANumeric number, HANumber *sender);
  static void onDisplayEnabledCommand(bool state, HASwitch *sender);
};

#endif
