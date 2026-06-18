#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <WebServer.h>

#include "DisplaySettings.h"
#include "HomeAssistantBridge.h"
#include "MatrixDisplay.h"
#include "MqttSettings.h"
#include "NetworkManager.h"
#include "RemoteClimateSource.h"
#include "TemperatureBoostSettings.h"

class WebInterface
{
public:
  WebInterface(NetworkManager &network,
               RemoteClimateSource &source,
               HomeAssistantBridge &homeAssistant,
               MqttSettings &mqttSettings,
               MatrixDisplay &display,
               DisplaySettings &displaySettings,
               TemperatureBoostSettings &boostSettings);

  void begin();
  void loop();

private:
  WebServer _server;
  NetworkManager &_network;
  RemoteClimateSource &_source;
  HomeAssistantBridge &_homeAssistant;
  MqttSettings &_mqttSettings;
  MatrixDisplay &_display;
  DisplaySettings &_displaySettings;
  TemperatureBoostSettings &_boostSettings;

  void setupRoutes();
  void handleRoot();
  void handleStatus();
  void handleWifiScan();
  void handleWifiSave();
  void handleWifiReset();
  void handleMqttSave();
  void handleMqttReset();
  void handleDisplaySave();
  void handleBoostSave();
  void sendOk();

  static bool parseIp(const String &text, IPAddress &address);
  static String jsonEscape(const String &value);
};

#endif
