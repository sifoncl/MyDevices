#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <WebServer.h>

#include "ClimateSensor.h"
#include "HomeAssistantBridge.h"
#include "MqttSettings.h"
#include "NetworkManager.h"
#include "PcPowerController.h"
#include "PcPowerSettings.h"
#include "RGBWWWController.h"

class WebInterface
{
public:
  WebInterface(NetworkManager &network,
               ClimateSensor &climate,
               RGBWWWController &lightController,
               PcPowerController &pcPower,
               PcPowerSettings &pcSettings,
               HomeAssistantBridge &homeAssistant,
               MqttSettings &mqttSettings);

  void begin();
  void loop();

private:
  WebServer _server;
  NetworkManager &_network;
  ClimateSensor &_climate;
  RGBWWWController &_lightController;
  PcPowerController &_pcPower;
  PcPowerSettings &_pcSettings;
  HomeAssistantBridge &_homeAssistant;
  MqttSettings &_mqttSettings;

  void setupRoutes();
  void handleRoot();
  void handleStatus();
  void handleWifiScan();
  void handleWifiSave();
  void handleWifiReset();
  void handleMqttSave();
  void handleMqttReset();
  void handleLightSave();
  void handlePcSettingsSave();
  void handlePcSettingsReset();
  void sendOk();

  static bool parseIp(const String &text, IPAddress &address);
  static String jsonEscape(const String &value);
  static bool argEnabled(WebServer &server, const char *name, bool fallback);
  static uint32_t argUInt(WebServer &server,
                          const char *name,
                          uint32_t fallback,
                          uint32_t minValue,
                          uint32_t maxValue);
};

#endif
