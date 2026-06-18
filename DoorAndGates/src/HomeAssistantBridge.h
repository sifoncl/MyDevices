#ifndef HOME_ASSISTANT_BRIDGE_H
#define HOME_ASSISTANT_BRIDGE_H

#include <ArduinoHA.h>
#include <WiFi.h>

#include "ClimateController.h"
#include "GateController.h"
#include "NetworkManager.h"

class HomeAssistantBridge
{
public:
  HomeAssistantBridge(GateController &gate,
                      ClimateController &climate,
                      NetworkManager &network,
                      const char *deviceId);

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

  GateController &_gate;
  ClimateController &_climate;
  NetworkManager &_network;

  WiFiClient _client;
  HADevice _device;
  HAMqtt _mqtt;
  HACover _cover;
  HAButton _wicketButton;
  HABinarySensor _wicketState;
  HALight _light;
  HASensorNumber _temperature;
  HASensorNumber _humidity;

  uint32_t _lastGateRevision = UINT32_MAX;
  unsigned long _lastPublishAt = 0;
  char _configurationUrl[64] = "";

  void publishAll(bool force);
  void publishGate(bool force);
  static void onMqttConnected();
  static void onCoverCommand(HACover::CoverCommand command, HACover *sender);
  static void onWicketCommand(HAButton *sender);
  static void onLightCommand(bool state, HALight *sender);
};

#endif
