#include <Arduino.h>

#include "AppConfig.h"
#include "ClimateSensor.h"
#include "HomeAssistantBridge.h"
#include "MqttSettings.h"
#include "NetworkManager.h"
#include "PcPowerController.h"
#include "PcPowerSettings.h"
#include "RGBWWWController.h"
#include "WebInterface.h"

NetworkManager network(
    Config::WIFI_DEFAULT_SSID,
    Config::WIFI_DEFAULT_PASSWORD,
    Config::WIFI_SETUP_AP_SSID_VALUE,
    Config::WIFI_SETUP_AP_PASSWORD_VALUE,
    Config::WIFI_HOSTNAME_VALUE,
    Config::WIFI_CONNECT_TIMEOUT_MS,
    Config::WIFI_RECONNECT_INTERVAL_MS,
    Config::wifiSetupApIp());

MqttSettings mqttSettings(
    Config::MQTT_SERVER_VALUE,
    Config::MQTT_PORT_VALUE,
    Config::MQTT_USER_VALUE,
    Config::MQTT_PASSWORD_VALUE);

PcPowerRuntimeSettings makeDefaultPcPowerSettings()
{
  PcPowerRuntimeSettings settings;
  settings.statusActiveHigh = Config::PC_STATUS_ACTIVE_HIGH;
  settings.buttonActiveHigh = Config::PC_POWER_BUTTON_ACTIVE_HIGH;
  settings.statusDebounceMs = Config::PC_STATUS_DEBOUNCE_MS;
  settings.buttonPulseMs = Config::PC_POWER_BUTTON_PULSE_MS;
  settings.startRetryMs = Config::PC_START_RETRY_MS;
  settings.shutdownRetryMs = Config::PC_SHUTDOWN_RETRY_MS;
  settings.maxStartAttempts = Config::PC_MAX_START_ATTEMPTS;
  settings.maxShutdownAttempts = Config::PC_MAX_SHUTDOWN_ATTEMPTS;
  return settings;
}

PcPowerRuntimeSettings defaultPcPowerSettings = makeDefaultPcPowerSettings();

PcPowerSettings pcPowerSettings(defaultPcPowerSettings);

PcPowerController pcPower(
    Config::PC_STATUS_PIN,
    Config::PC_POWER_BUTTON_PIN,
    Config::PC_STATUS_INPUT_MODE,
    pcPowerSettings);

RGBWWWController ledStrip(
    Config::RED_PIN,
    Config::GREEN_PIN,
    Config::BLUE_PIN,
    Config::WHITE_PIN,
    Config::WARM_WHITE_PIN);

ClimateSensor climate(
    Config::DHT_DATA_PIN,
    Config::DHT_TYPE_VALUE,
    Config::SENSOR_READ_INTERVAL_MS,
    Config::SENSOR_INVALID_TIMEOUT_MS);

HomeAssistantBridge homeAssistant(
    climate,
    ledStrip,
    pcPower,
    network,
    Config::DEVICE_ID,
    Config::HA_PUBLISH_INTERVAL_MS);

WebInterface web(
    network,
    climate,
    ledStrip,
    pcPower,
    pcPowerSettings,
    homeAssistant,
    mqttSettings);

namespace
{
uint32_t lastTouchChangeAt = 0;
bool touchState = false;
bool lastTouchRaw = false;

void setupLightDefaults()
{
  ledStrip.setTempParams(
      Config::MAX_COLOR_TEMP_MIRED,
      Config::MIN_COLOR_TEMP_MIRED,
      true);
  ledStrip.setWWCWmode();
  ledStrip.setBrightness(Config::DEFAULT_LIGHT_BRIGHTNESS);
  ledStrip.setColorTempAndChangeMode(Config::DEFAULT_LIGHT_COLOR_TEMP_MIRED);
  ledStrip.turnOff();
}

void touchButtonLoop()
{
  const uint16_t touchValue = touchRead(Config::TOUCH_BUTTON_PIN);
  const bool rawTouched = touchValue < Config::TOUCH_THRESHOLD;
  const uint32_t now = millis();

  if (rawTouched != lastTouchRaw)
  {
    lastTouchRaw = rawTouched;
    lastTouchChangeAt = now;
  }

  if ((now - lastTouchChangeAt) < Config::TOUCH_DEBOUNCE_MS)
  {
    return;
  }

  if (rawTouched == touchState)
  {
    return;
  }

  touchState = rawTouched;
  if (!touchState)
  {
    return;
  }

  if (ledStrip.isOn())
  {
    ledStrip.turnOff();
  }
  else
  {
    ledStrip.turnOn();
  }
}
}

void setup()
{
  Serial.begin(Config::SERIAL_BAUD);
  delay(300);

  Serial.println();
  Serial.print(Config::DEVICE_NAME);
  Serial.print(" v");
  Serial.println(Config::SOFTWARE_VERSION);

  mqttSettings.begin();
  pcPowerSettings.begin();

  setupLightDefaults();
  pcPower.begin();
  climate.begin();

  network.begin();
  homeAssistant.begin(
      mqttSettings.server().c_str(),
      mqttSettings.port(),
      mqttSettings.user().isEmpty() ? nullptr : mqttSettings.user().c_str(),
      mqttSettings.password().isEmpty() ? nullptr : mqttSettings.password().c_str());
  web.begin();

  Serial.println("PC Lamp controller started");
}

void loop()
{
  network.loop();
  web.loop();
  climate.loop();
  touchButtonLoop();
  pcPower.loop();
  homeAssistant.loop();
}
