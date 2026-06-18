#include "HomeAssistantBridge.h"

#include "AppConfig.h"

HomeAssistantBridge *HomeAssistantBridge::_instance = nullptr;

namespace
{
const char *relayName(uint8_t index)
{
  switch (index)
  {
  case 0:
    return "Valve 1";
  case 1:
    return "Valve 2";
  case 2:
    return "Pump";
  case 3:
    return "Reserve";
  default:
    return "Relay";
  }
}

const char *relayIcon(uint8_t index)
{
  switch (index)
  {
  case 0:
  case 1:
    return "mdi:valve";
  case 2:
    return "mdi:pump";
  default:
    return "mdi:electric-switch";
  }
}
}

HomeAssistantBridge::HomeAssistantBridge(RelayController &relays,
                                         ClimateSensor &climate,
                                         PressureSensor &pressure,
                                         NetworkManager &network,
                                         const char *deviceId,
                                         uint32_t publishIntervalMs)
    : _relays(relays),
      _climate(climate),
      _pressure(pressure),
      _network(network),
      _publishIntervalMs(publishIntervalMs),
      _device(deviceId),
      _mqtt(_client, _device),
      _relay1("valve1"),
      _relay2("valve2"),
      _relay3("pump"),
      _relay4("reserv"),
      _temperature("WhaterRoomTemp", HASensorNumber::PrecisionP1),
      _humidity("WhaterRoomHumd", HASensorNumber::PrecisionP1),
      _pressureSensor("WhaterRoomPressure", HASensorNumber::PrecisionP1)
{
  _relaySwitches[0] = &_relay1;
  _relaySwitches[1] = &_relay2;
  _relaySwitches[2] = &_relay3;
  _relaySwitches[3] = &_relay4;
  _instance = this;
}

void HomeAssistantBridge::begin(const char *mqttServer,
                                uint16_t mqttPort,
                                const char *mqttUser,
                                const char *mqttPassword)
{
  _client.setTimeout(1);
  configureDevice();
  configureEntities();
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

  const bool relayChanged = _lastRelayRevision != _relays.revision();
  const bool climateChanged = _lastClimateRevision != _climate.revision();
  const bool pressureChanged = _lastPressureRevision != _pressure.revision();
  const bool periodic = (millis() - _lastPublishAt) >= _publishIntervalMs;
  if (!relayChanged && !climateChanged && !pressureChanged && !periodic)
  {
    return;
  }

  _lastRelayRevision = _relays.revision();
  _lastClimateRevision = _climate.revision();
  _lastPressureRevision = _pressure.revision();
  _lastPublishAt = millis();
  publishAll(false);
}

bool HomeAssistantBridge::connected() const
{
  return _mqtt.isConnected();
}

void HomeAssistantBridge::configureDevice()
{
  _device.setName(Config::DEVICE_NAME);
  _device.setManufacturer("Custom");
  _device.setModel("ESP32-S3 water controller");
  _device.setSoftwareVersion(Config::SOFTWARE_VERSION);
  _device.enableExtendedUniqueIds();
  updateConfigurationUrl();
}

void HomeAssistantBridge::configureEntities()
{
  for (uint8_t i = 0; i < RELAY_COUNT; i++)
  {
    _relaySwitches[i]->setName(relayName(i));
    _relaySwitches[i]->setIcon(relayIcon(i));
    _relaySwitches[i]->onCommand(onRelayCommand);
  }

  _temperature.setName("Pump room temperature");
  _temperature.setIcon("mdi:thermometer");
  _temperature.setUnitOfMeasurement("C");
  _temperature.setDeviceClass("temperature");
  _temperature.setStateClass("measurement");

  _humidity.setName("Pump room humidity");
  _humidity.setIcon("mdi:water-percent");
  _humidity.setUnitOfMeasurement("%");
  _humidity.setDeviceClass("humidity");
  _humidity.setStateClass("measurement");

  _pressureSensor.setName("Atmospheric pressure");
  _pressureSensor.setIcon("mdi:speedometer-medium");
  _pressureSensor.setUnitOfMeasurement("mmHg");
  _pressureSensor.setDeviceClass("pressure");
  _pressureSensor.setStateClass("measurement");
}

void HomeAssistantBridge::updateConfigurationUrl()
{
  snprintf(_configurationUrl,
           sizeof(_configurationUrl),
           "http://%s/",
           _network.address().toString().c_str());
  _device.setConfigurationUrl(_configurationUrl);
}

void HomeAssistantBridge::publishAll(bool force)
{
  publishRelays(force);
  publishClimate(force);
  publishPressure(force);
}

void HomeAssistantBridge::publishRelays(bool force)
{
  for (uint8_t i = 0; i < RELAY_COUNT && i < _relays.count(); i++)
  {
    _relaySwitches[i]->setState(_relays.state(i), force);
  }
}

void HomeAssistantBridge::publishClimate(bool force)
{
  const bool valid = _climate.valid();
  _temperature.setAvailability(valid);
  _humidity.setAvailability(valid);

  if (valid)
  {
    _temperature.setValue(_climate.temperature(), force);
    _humidity.setValue(_climate.humidity(), force);
  }
}

void HomeAssistantBridge::publishPressure(bool force)
{
  const bool valid = _pressure.valid();
  _pressureSensor.setAvailability(valid);

  if (valid)
  {
    _pressureSensor.setValue(_pressure.pressureMmHg(), force);
  }
}

void HomeAssistantBridge::onMqttConnected()
{
  if (_instance == nullptr)
  {
    return;
  }

  _instance->updateConfigurationUrl();
  _instance->publishAll(true);
}

void HomeAssistantBridge::onRelayCommand(bool state, HASwitch *sender)
{
  if (_instance == nullptr)
  {
    return;
  }

  for (uint8_t i = 0; i < RELAY_COUNT; i++)
  {
    if (sender == _instance->_relaySwitches[i])
    {
      _instance->_relays.set(i, state);
      sender->setState(state);
      Serial.print("Relay ");
      Serial.print(i + 1);
      Serial.print(" set from HA: ");
      Serial.println(state ? "on" : "off");
      return;
    }
  }
}
