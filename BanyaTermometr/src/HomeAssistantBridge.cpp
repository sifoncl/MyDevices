#include "HomeAssistantBridge.h"

#include "AppConfig.h"

HomeAssistantBridge *HomeAssistantBridge::_instance = nullptr;

HomeAssistantBridge::HomeAssistantBridge(ClimateSensor &climate,
                                         MatrixDisplay &display,
                                         DisplaySettings &displaySettings,
                                         NetworkManager &network,
                                         const char *deviceId,
                                         uint32_t publishIntervalMs)
    : _climate(climate),
      _display(display),
      _displaySettings(displaySettings),
      _network(network),
      _device(deviceId),
      _mqtt(_client, _device),
      _temperature("BanyaTemp", HASensorNumber::PrecisionP1),
      _humidity("BanyaHumid", HASensorNumber::PrecisionP1),
      _displayBrightness("DisplayBrightness", HANumber::PrecisionP0),
      _displayEnabled("DisplayEnabled"),
      _publishIntervalMs(publishIntervalMs)
{
  _instance = this;
}

void HomeAssistantBridge::begin(const char *mqttServer,
                                uint16_t mqttPort,
                                const char *mqttUser,
                                const char *mqttPassword)
{
  _client.setTimeout(1);

  _device.setName(Config::DEVICE_NAME);
  _device.setManufacturer("Custom");
  _device.setModel("ESP32-S3 Super Mini with SHT85");
  _device.setSoftwareVersion(Config::SOFTWARE_VERSION);
  _device.enableExtendedUniqueIds();

  snprintf(_configurationUrl,
           sizeof(_configurationUrl),
           "http://%s/",
           _network.address().toString().c_str());
  _device.setConfigurationUrl(_configurationUrl);

  _temperature.setName("Температура в парилке");
  _temperature.setIcon("mdi:thermometer");
  _temperature.setUnitOfMeasurement("°C");
  _temperature.setDeviceClass("temperature");
  _temperature.setStateClass("measurement");

  _humidity.setName("Влажность в парилке");
  _humidity.setIcon("mdi:water-percent");
  _humidity.setUnitOfMeasurement("%");
  _humidity.setDeviceClass("humidity");
  _humidity.setStateClass("measurement");

  _displayBrightness.setName("Яркость дисплея");
  _displayBrightness.setIcon("mdi:brightness-6");
  _displayBrightness.setMin(Config::MATRIX_MIN_INTENSITY);
  _displayBrightness.setMax(Config::MATRIX_MAX_INTENSITY);
  _displayBrightness.setStep(1);
  _displayBrightness.setMode(HANumber::ModeSlider);
  _displayBrightness.setRetain(true);
  _displayBrightness.setCurrentState(_display.intensity());
  _displayBrightness.onCommand(onBrightnessCommand);

  _displayEnabled.setName("Экран");
  _displayEnabled.setIcon("mdi:led-strip");
  _displayEnabled.setRetain(true);
  _displayEnabled.setCurrentState(_display.enabled());
  _displayEnabled.onCommand(onDisplayEnabledCommand);

  _mqtt.onConnected(onMqttConnected);
  _mqtt.begin(mqttServer, mqttPort, mqttUser, mqttPassword);
}

bool HomeAssistantBridge::reconfigure(const char *mqttServer,
                                      uint16_t mqttPort,
                                      const char *mqttUser,
                                      const char *mqttPassword)
{
  _mqtt.disconnect();
  return _mqtt.begin(mqttServer, mqttPort, mqttUser, mqttPassword);
}

void HomeAssistantBridge::loop()
{
  _mqtt.loop();

  const uint32_t now = millis();
  const bool changed = _lastClimateRevision != _climate.revision();
  const bool periodic = (now - _lastPublishAt) >= _publishIntervalMs;
  if (!changed && !periodic)
  {
    return;
  }

  _lastClimateRevision = _climate.revision();
  _lastPublishAt = now;
  publish(false);
}

bool HomeAssistantBridge::connected() const
{
  return _mqtt.isConnected();
}

void HomeAssistantBridge::publish(bool force)
{
  const bool sensorAvailable = _climate.valid();
  _temperature.setAvailability(sensorAvailable);
  _humidity.setAvailability(sensorAvailable);
  _displayBrightness.setAvailability(true);
  _displayEnabled.setAvailability(true);
  _displayBrightness.setState(_display.intensity(), force);
  _displayEnabled.setState(_display.enabled(), force);
  _mqtt.publish(Config::MIRROR_MQTT_VALID_TOPIC,
                sensorAvailable ? "1" : "0",
                true);

  if (sensorAvailable)
  {
    _temperature.setValue(_climate.temperature(), force);
    _humidity.setValue(_climate.humidity(), force);

    char value[16];
    snprintf(value, sizeof(value), "%.2f", _climate.temperature());
    _mqtt.publish(Config::MIRROR_MQTT_TEMPERATURE_TOPIC, value, true);

    snprintf(value, sizeof(value), "%.2f", _climate.humidity());
    _mqtt.publish(Config::MIRROR_MQTT_HUMIDITY_TOPIC, value, true);
  }
}

void HomeAssistantBridge::onMqttConnected()
{
  if (_instance != nullptr)
  {
    _instance->publish(true);
  }
}

void HomeAssistantBridge::onBrightnessCommand(HANumeric number,
                                              HANumber *sender)
{
  if (_instance == nullptr || !number.isSet())
  {
    return;
  }

  int32_t value = number.toInt32();
  if (value < Config::MATRIX_MIN_INTENSITY)
  {
    value = Config::MATRIX_MIN_INTENSITY;
  }
  if (value > Config::MATRIX_MAX_INTENSITY)
  {
    value = Config::MATRIX_MAX_INTENSITY;
  }

  const uint8_t intensity = static_cast<uint8_t>(value);
  _instance->_displaySettings.saveBrightness(intensity);
  _instance->_display.setIntensity(intensity);

  if (sender != nullptr)
  {
    sender->setState(intensity, true);
  }
}

void HomeAssistantBridge::onDisplayEnabledCommand(bool state,
                                                  HASwitch *sender)
{
  if (_instance == nullptr)
  {
    return;
  }

  _instance->_displaySettings.saveEnabled(state);
  _instance->_display.setEnabled(state);

  if (sender != nullptr)
  {
    sender->setState(_instance->_display.enabled(), true);
  }
}
