#ifndef HOME_ASSISTANT_BRIDGE_H
#define HOME_ASSISTANT_BRIDGE_H

#include <ArduinoHA.h>
#include <WiFi.h>

#include "ClimateSensor.h"
#include "NetworkManager.h"
#include "PressureSensor.h"
#include "RelayController.h"

class HomeAssistantBridge
{
public:
  HomeAssistantBridge(RelayController &relays,
                      ClimateSensor &climate,
                      PressureSensor &pressure,
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
  static constexpr uint8_t RELAY_COUNT = 4;
  static HomeAssistantBridge *_instance;

  RelayController &_relays;
  ClimateSensor &_climate;
  PressureSensor &_pressure;
  NetworkManager &_network;
  uint32_t _publishIntervalMs;

  WiFiClient _client;
  HADevice _device;
  HAMqtt _mqtt;
  HASwitch _relay1;
  HASwitch _relay2;
  HASwitch _relay3;
  HASwitch _relay4;
  HASwitch *_relaySwitches[RELAY_COUNT] = {};
  HASensorNumber _temperature;
  HASensorNumber _humidity;
  HASensorNumber _pressureSensor;

  uint32_t _lastRelayRevision = UINT32_MAX;
  uint32_t _lastClimateRevision = UINT32_MAX;
  uint32_t _lastPressureRevision = UINT32_MAX;
  uint32_t _lastPublishAt = 0;
  char _configurationUrl[64] = "";

  void configureDevice();
  void configureEntities();
  void updateConfigurationUrl();
  void publishAll(bool force);
  void publishRelays(bool force);
  void publishClimate(bool force);
  void publishPressure(bool force);
  static void onMqttConnected();
  static void onRelayCommand(bool state, HASwitch *sender);
};

#endif
