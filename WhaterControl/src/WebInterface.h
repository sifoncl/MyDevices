#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <WebServer.h>

#include "ClimateSensor.h"
#include "HomeAssistantBridge.h"
#include "MqttSettings.h"
#include "NetworkManager.h"
#include "PressureSensor.h"
#include "RelayController.h"

class WebInterface
{
public:
  WebInterface(NetworkManager &network,
               RelayController &relays,
               ClimateSensor &climate,
               PressureSensor &pressure,
               HomeAssistantBridge &homeAssistant,
               MqttSettings &mqttSettings);

  void begin();
  void loop();

private:
  WebServer _server;
  NetworkManager &_network;
  RelayController &_relays;
  ClimateSensor &_climate;
  PressureSensor &_pressure;
  HomeAssistantBridge &_homeAssistant;
  MqttSettings &_mqttSettings;

  void setupRoutes();
  void handleRoot();
  void handleStatus();
  void handleRelaySet();
  void handleRelayToggle();
  void handleWifiScan();
  void handleWifiSave();
  void handleWifiReset();
  void handleMqttSave();
  void handleMqttReset();
  void sendOk();
  void sendError(uint16_t code, const char *message);

  static String jsonEscape(const String &value);
  static bool parseIp(const String &value, IPAddress &address);
  static const char *relayName(uint8_t index);
};

#endif
