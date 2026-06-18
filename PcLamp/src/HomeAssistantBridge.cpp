#include "HomeAssistantBridge.h"

#include "AppConfig.h"

HomeAssistantBridge *HomeAssistantBridge::_instance = nullptr;

HomeAssistantBridge::HomeAssistantBridge(ClimateSensor &climate,
                                         RGBWWWController &lightController,
                                         PcPowerController &pcPower,
                                         NetworkManager &network,
                                         const char *deviceId,
                                         uint32_t publishIntervalMs)
    : _climate(climate),
      _lightController(lightController),
      _pcPower(pcPower),
      _network(network),
      _device(deviceId),
      _mqtt(_client, _device),
      _light("pc_lamp", HALight::BrightnessFeature | HALight::ColorTemperatureFeature | HALight::RGBFeature),
      _pcSwitch("pc_power"),
      _temperature("bedroom_temperature", HASensorNumber::PrecisionP1),
      _humidity("bedroom_humidity", HASensorNumber::PrecisionP1),
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
  _device.setModel("ESP32 PC lamp and power button controller");
  _device.setSoftwareVersion(Config::SOFTWARE_VERSION);
  _device.enableExtendedUniqueIds();

  snprintf(_configurationUrl,
           sizeof(_configurationUrl),
           "http://%s/",
           _network.address().toString().c_str());
  _device.setConfigurationUrl(_configurationUrl);

  _light.setName("Свет над ПК");
  _light.setIcon("mdi:desk-lamp-on");
  _light.setBrightnessScale(255);
  _light.setMinMireds(Config::MIN_COLOR_TEMP_MIRED);
  _light.setMaxMireds(Config::MAX_COLOR_TEMP_MIRED);
  _light.setCurrentState(_lightController.isOn());
  _light.setCurrentBrightness(_lightController.getBrightness());
  _light.setCurrentColorTemperature(_lightController.getColorTemperature());
  _light.setCurrentRGBColor(HALight::RGBColor(
      _lightController.getRed(),
      _lightController.getGreen(),
      _lightController.getBlue()));
  _light.onStateCommand(onLightStateCommand);
  _light.onBrightnessCommand(onBrightnessCommand);
  _light.onColorTemperatureCommand(onColorTemperatureCommand);
  _light.onRGBColorCommand(onRgbCommand);

  _pcSwitch.setName("Kolya PC");
  _pcSwitch.setIcon("mdi:desktop-classic");
  _pcSwitch.setCurrentState(_pcPower.isOn());
  _pcSwitch.onCommand(onPcSwitchCommand);

  _temperature.setName("Температура в спальне");
  _temperature.setIcon("mdi:thermometer");
  _temperature.setUnitOfMeasurement("°C");
  _temperature.setDeviceClass("temperature");
  _temperature.setStateClass("measurement");

  _humidity.setName("Влажность в спальне");
  _humidity.setIcon("mdi:water-percent");
  _humidity.setUnitOfMeasurement("%");
  _humidity.setDeviceClass("humidity");
  _humidity.setStateClass("measurement");

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
  const bool changed = _lastClimateRevision != _climate.revision() ||
                       _lastLightRevision != _lightController.revision() ||
                       _lastPcRevision != _pcPower.revision();
  const bool periodic = (now - _lastPublishAt) >= _publishIntervalMs;
  if (!changed && !periodic)
  {
    return;
  }

  _lastClimateRevision = _climate.revision();
  _lastLightRevision = _lightController.revision();
  _lastPcRevision = _pcPower.revision();
  _lastPublishAt = now;
  publishAll(false);
}

bool HomeAssistantBridge::connected() const
{
  return _mqtt.isConnected();
}

void HomeAssistantBridge::publishAll(bool force)
{
  _light.setState(_lightController.isOn(), force);
  _light.setBrightness(_lightController.getBrightness(), force);
  _light.setColorTemperature(_lightController.getColorTemperature(), force);
  _light.setRGBColor(HALight::RGBColor(
                         _lightController.getRed(),
                         _lightController.getGreen(),
                         _lightController.getBlue()),
                     force);

  _pcSwitch.setState(_pcPower.isOn(), force);

  const bool sensorAvailable = _climate.valid();
  _temperature.setAvailability(sensorAvailable);
  _humidity.setAvailability(sensorAvailable);

  if (sensorAvailable)
  {
    _temperature.setValue(_climate.temperature(), force);
    _humidity.setValue(_climate.humidity(), force);
  }
}

void HomeAssistantBridge::onMqttConnected()
{
  if (_instance != nullptr)
  {
    _instance->publishAll(true);
  }
}

void HomeAssistantBridge::onLightStateCommand(bool state, HALight *sender)
{
  if (_instance == nullptr)
  {
    return;
  }

  if (state)
  {
    _instance->_lightController.turnOn();
  }
  else
  {
    _instance->_lightController.turnOff();
  }

  if (sender != nullptr)
  {
    sender->setState(_instance->_lightController.isOn(), true);
  }
}

void HomeAssistantBridge::onBrightnessCommand(uint8_t brightness, HALight *sender)
{
  if (_instance == nullptr)
  {
    return;
  }

  _instance->_lightController.setBrightness(brightness);

  if (sender != nullptr)
  {
    sender->setBrightness(_instance->_lightController.getBrightness(), true);
  }
}

void HomeAssistantBridge::onColorTemperatureCommand(uint16_t temperature, HALight *sender)
{
  if (_instance == nullptr)
  {
    return;
  }

  _instance->_lightController.setColorTempAndChangeMode(temperature);

  if (sender != nullptr)
  {
    sender->setColorTemperature(_instance->_lightController.getColorTemperature(), true);
  }
}

void HomeAssistantBridge::onRgbCommand(HALight::RGBColor color, HALight *sender)
{
  if (_instance == nullptr || !color.isSet)
  {
    return;
  }

  _instance->_lightController.setRGBandChageMode(color.red, color.green, color.blue);

  if (sender != nullptr)
  {
    sender->setRGBColor(color, true);
    sender->setState(_instance->_lightController.isOn(), true);
  }
}

void HomeAssistantBridge::onPcSwitchCommand(bool state, HASwitch *sender)
{
  if (_instance == nullptr)
  {
    return;
  }

  if (state)
  {
    _instance->_pcPower.commandTurnOn();
  }
  else
  {
    _instance->_pcPower.commandTurnOff();
  }

  if (sender != nullptr)
  {
    sender->setState(_instance->_pcPower.isOn(), true);
  }
}
