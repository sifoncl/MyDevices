#ifndef HOME_ASSISTANT_BRIDGE_H
#define HOME_ASSISTANT_BRIDGE_H

#include <ArduinoHA.h>
#include <WiFi.h>

#include "ClimateSensor.h"
#include "NetworkManager.h"
#include "PcPowerController.h"
#include "RGBWWWController.h"

class HomeAssistantBridge
{
public:
  HomeAssistantBridge(ClimateSensor &climate,
                      RGBWWWController &lightController,
                      PcPowerController &pcPower,
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
  RGBWWWController &_lightController;
  PcPowerController &_pcPower;
  NetworkManager &_network;

  WiFiClient _client;
  HADevice _device;
  HAMqtt _mqtt;
  HALight _light;
  HASwitch _pcSwitch;
  HASensorNumber _temperature;
  HASensorNumber _humidity;

  uint32_t _publishIntervalMs;
  uint32_t _lastPublishAt = 0;
  uint32_t _lastClimateRevision = UINT32_MAX;
  uint32_t _lastLightRevision = UINT32_MAX;
  uint32_t _lastPcRevision = UINT32_MAX;
  char _configurationUrl[64] = "";

  void publishAll(bool force);
  static void onMqttConnected();
  static void onLightStateCommand(bool state, HALight *sender);
  static void onBrightnessCommand(uint8_t brightness, HALight *sender);
  static void onColorTemperatureCommand(uint16_t temperature, HALight *sender);
  static void onRgbCommand(HALight::RGBColor color, HALight *sender);
  static void onPcSwitchCommand(bool state, HASwitch *sender);
};

#endif
