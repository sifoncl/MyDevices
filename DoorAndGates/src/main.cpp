#include <Arduino.h>
#include <Wire.h>

#include "AppConfig.h"
#include "ClimateController.h"
#include "DisplaySettings.h"
#include "DisplayController.h"
#include "GateController.h"
#include "HomeAssistantBridge.h"
#include "InputController.h"
#include "MagnetSensorController.h"
#include "MqttSettings.h"
#include "NetworkManager.h"
#include "WebInterface.h"

NetworkManager network(
    WIFI_SSID,
    WIFI_PASSWORD,
    WIFI_SETUP_AP_SSID,
    WIFI_SETUP_AP_PASSWORD,
    WIFI_HOSTNAME,
    WIFI_CONNECT_TIMEOUT_MS,
    WIFI_RECONNECT_INTERVAL_MS);

GateController gate(
    GATE_OPEN_RELAY_PIN,
    GATE_CLOSE_RELAY_PIN,
    WICKET_RELAY_PIN,
    LIGHT_RELAY_PIN,
    RELAY_ACTIVE_LEVEL,
    GATE_RELAY_SWITCH_GUARD_MS,
    WICKET_PULSE_MS);

const uint8_t upperSensorMac[6] = UPPER_SENSOR_MAC;
const uint8_t lowerSensorMac[6] = LOWER_SENSOR_MAC;

MagnetSensorController magnets(
    upperSensorMac,
    lowerSensorMac,
    REMOTE_SENSOR_TIMEOUT_MS);

ClimateController climate(SHT31_ADDRESS, SHT31_READ_INTERVAL_MS);

DisplayController screen(
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    Wire,
    OLED_ADDRESS,
    DISPLAY_RENDER_INTERVAL_MS,
    IP_SCREEN_MS);

MqttSettings mqttSettings(
    MQTT_SERVER,
    MQTT_PORT,
    MQTT_USER,
    MQTT_PASSWORD);

DisplaySettings displaySettings(
    DISPLAY_BRIGHTNESS,
    DISPLAY_MIN_BRIGHTNESS,
    DISPLAY_MAX_BRIGHTNESS);

InputController inputs(gate);
HomeAssistantBridge homeAssistant(gate, climate, network, DEVICE_ID);
WebInterface web(
    network,
    gate,
    magnets,
    climate,
    homeAssistant,
    mqttSettings,
    screen,
    displaySettings);

void setup()
{
  Serial.begin(SERIAL_BAUD);
  delay(300);

  mqttSettings.begin();
  displaySettings.begin();

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  gate.begin();
  climate.begin();
  screen.begin();
  screen.setBrightness(static_cast<uint8_t>(displaySettings.brightness()));
  inputs.begin();

  network.begin();
  magnets.begin();
  homeAssistant.begin(
      mqttSettings.server().c_str(),
      mqttSettings.port(),
      mqttSettings.user().isEmpty() ? nullptr : mqttSettings.user().c_str(),
      mqttSettings.password().isEmpty() ? nullptr : mqttSettings.password().c_str());
  web.begin();
  screen.showIp(network);

  Serial.println("Door and Gates controller started");
}

void loop()
{
  network.loop();
  web.loop();

  if (magnets.loop())
  {
    gate.processMagnetStates(magnets.upperState(), magnets.lowerState());
  }

  climate.loop();
  inputs.loop();
  gate.loop();
  homeAssistant.loop();
  screen.loop(climate, network);
}
