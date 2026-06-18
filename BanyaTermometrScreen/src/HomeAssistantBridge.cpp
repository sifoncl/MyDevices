#include "HomeAssistantBridge.h"

#include "AppConfig.h"

HomeAssistantBridge *HomeAssistantBridge::_instance = nullptr;

HomeAssistantBridge::HomeAssistantBridge(RemoteClimateSource &source,
                                         TemperatureBoostSettings &boost,
                                         MatrixDisplay &display,
                                         DisplaySettings &displaySettings,
                                         NetworkManager &network,
                                         const char *deviceId,
                                         uint32_t publishIntervalMs)
    : _source(source),
      _boost(boost),
      _display(display),
      _displaySettings(displaySettings),
      _network(network),
      _device(deviceId),
      _mqtt(_client, _device),
      _temperature("ScreenTemp", HASensorNumber::PrecisionP1),
      _rawTemperature("RawTemp", HASensorNumber::PrecisionP1),
      _humidity("ScreenHumid", HASensorNumber::PrecisionP1),
      _sourceName("DataSource"),
      _displayBrightness("DisplayBrightness", HANumber::PrecisionP0),
      _displayEnabled("DisplayEnabled"),
      _boostFactor("TemperatureBoostFactor", HANumber::PrecisionP2),
      _boostEnabled("TemperatureBoostEnabled"),
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
  _device.setModel("ESP32-S3 Super Mini duplicate screen");
  _device.setSoftwareVersion(Config::SOFTWARE_VERSION);
  _device.enableExtendedUniqueIds();

  snprintf(_configurationUrl,
           sizeof(_configurationUrl),
           "http://%s/",
           _network.address().toString().c_str());
  _device.setConfigurationUrl(_configurationUrl);

  _temperature.setName("Температура табло");
  _temperature.setIcon("mdi:thermometer");
  _temperature.setUnitOfMeasurement("°C");
  _temperature.setDeviceClass("temperature");
  _temperature.setStateClass("measurement");

  _rawTemperature.setName("Фактическая температура");
  _rawTemperature.setIcon("mdi:thermometer-lines");
  _rawTemperature.setUnitOfMeasurement("°C");
  _rawTemperature.setDeviceClass("temperature");
  _rawTemperature.setStateClass("measurement");

  _humidity.setName("Влажность табло");
  _humidity.setIcon("mdi:water-percent");
  _humidity.setUnitOfMeasurement("%");
  _humidity.setDeviceClass("humidity");
  _humidity.setStateClass("measurement");

  _sourceName.setName("Источник данных");
  _sourceName.setIcon("mdi:access-point-network");

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

  _boostFactor.setName("Коэффициент завышения температуры");
  _boostFactor.setIcon("mdi:thermometer-chevron-up");
  _boostFactor.setMin(Config::TEMPERATURE_BOOST_MIN_FACTOR);
  _boostFactor.setMax(Config::TEMPERATURE_BOOST_MAX_FACTOR);
  _boostFactor.setStep(Config::TEMPERATURE_BOOST_STEP);
  _boostFactor.setMode(HANumber::ModeSlider);
  _boostFactor.setRetain(true);
  _boostFactor.setCurrentState(_boost.factor());
  _boostFactor.onCommand(onBoostFactorCommand);

  _boostEnabled.setName("Завышение температуры");
  _boostEnabled.setIcon("mdi:tune-variant");
  _boostEnabled.setRetain(true);
  _boostEnabled.setCurrentState(_boost.enabled());
  _boostEnabled.onCommand(onBoostEnabledCommand);

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

  const ClimateReading reading = _source.current(_boost);
  const uint32_t now = millis();
  const bool changed = _lastRevision != reading.revision;
  const bool periodic = (now - _lastPublishAt) >= _publishIntervalMs;
  if (!changed && !periodic)
  {
    return;
  }

  _lastRevision = reading.revision;
  _lastPublishAt = now;
  publish(false);
}

bool HomeAssistantBridge::connected() const
{
  return _mqtt.isConnected();
}

void HomeAssistantBridge::publish(bool force)
{
  const ClimateReading reading = _source.current(_boost);
  _temperature.setAvailability(reading.valid);
  _rawTemperature.setAvailability(reading.valid);
  _humidity.setAvailability(reading.valid);
  _sourceName.setAvailability(true);
  _displayBrightness.setAvailability(true);
  _displayEnabled.setAvailability(true);
  _boostFactor.setAvailability(true);
  _boostEnabled.setAvailability(true);

  _displayBrightness.setState(_display.intensity(), force);
  _displayEnabled.setState(_display.enabled(), force);
  _boostFactor.setState(_boost.factor(), force);
  _boostEnabled.setState(_boost.enabled(), force);
  _sourceName.setValue(climateSourceName(reading.source));

  if (reading.valid)
  {
    _temperature.setValue(reading.displayTemperature, force);
    _rawTemperature.setValue(reading.rawTemperature, force);
    _humidity.setValue(reading.humidity, force);
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
  value = constrain(value,
                    static_cast<int32_t>(Config::MATRIX_MIN_INTENSITY),
                    static_cast<int32_t>(Config::MATRIX_MAX_INTENSITY));
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

void HomeAssistantBridge::onBoostFactorCommand(HANumeric number,
                                               HANumber *sender)
{
  if (_instance == nullptr || !number.isSet())
  {
    return;
  }

  _instance->_boost.save(_instance->_boost.enabled(), number.toFloat());
  if (sender != nullptr)
  {
    sender->setState(_instance->_boost.factor(), true);
  }
  _instance->publish(true);
}

void HomeAssistantBridge::onBoostEnabledCommand(bool state, HASwitch *sender)
{
  if (_instance == nullptr)
  {
    return;
  }

  _instance->_boost.save(state, _instance->_boost.factor());
  if (sender != nullptr)
  {
    sender->setState(_instance->_boost.enabled(), true);
  }
  _instance->publish(true);
}
