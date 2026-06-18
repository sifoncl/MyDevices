#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <WebServer.h>

#include "ClimateSensor.h"
#include "DisplaySettings.h"
#include "HomeAssistantBridge.h"
#include "MatrixDisplay.h"
#include "MqttSettings.h"
#include "NetworkManager.h"

class WebInterface
{
public:
  WebInterface(NetworkManager &network,
               ClimateSensor &climate,
               HomeAssistantBridge &homeAssistant,
               MqttSettings &mqttSettings,
               MatrixDisplay &display,
               DisplaySettings &displaySettings);

  void begin();
  void loop();

private:
  WebServer _server;
  NetworkManager &_network;
  ClimateSensor &_climate;
  HomeAssistantBridge &_homeAssistant;
  MqttSettings &_mqttSettings;
  MatrixDisplay &_display;
  DisplaySettings &_displaySettings;

  void setupRoutes();
  void handleRoot();
  void handleStatus();
  void handleWifiScan();
  void handleWifiSave();
  void handleWifiReset();
  void handleMqttSave();
  void handleMqttReset();
  void handleDisplaySave();
  void sendOk();

  static bool parseIp(const String &text, IPAddress &address);
  static String jsonEscape(const String &value);
};

#endif
