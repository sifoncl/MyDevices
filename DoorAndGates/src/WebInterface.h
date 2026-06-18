#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <WebServer.h>

#include "ClimateController.h"
#include "DisplayController.h"
#include "DisplaySettings.h"
#include "GateController.h"
#include "HomeAssistantBridge.h"
#include "MagnetSensorController.h"
#include "MqttSettings.h"
#include "NetworkManager.h"

class WebInterface
{
public:
  WebInterface(NetworkManager &network,
               GateController &gate,
               MagnetSensorController &magnets,
               ClimateController &climate,
               HomeAssistantBridge &homeAssistant,
               MqttSettings &mqttSettings,
               DisplayController &display,
               DisplaySettings &displaySettings);

  void begin();
  void loop();

private:
  WebServer _server;
  NetworkManager &_network;
  GateController &_gate;
  MagnetSensorController &_magnets;
  ClimateController &_climate;
  HomeAssistantBridge &_homeAssistant;
  MqttSettings &_mqttSettings;
  DisplayController &_display;
  DisplaySettings &_displaySettings;

  void setupRoutes();
  void handleRoot();
  void handleStatus();
  void handleSettings();
  void handleWifiScan();
  void handleWifiSave();
  void handleMqttSave();
  void handleMqttReset();
  void handleDisplaySave();
  void sendOk();

  static void appendSensorJson(String &json,
                               const char *name,
                               const MagneticPresenceSensorState &state,
                               const RemoteSensorLinkStatus &link);
  static String jsonEscape(const String &value);
  static String formatMac(const uint8_t mac[6]);
  static const char *directionText(int8_t direction);
};

#endif
