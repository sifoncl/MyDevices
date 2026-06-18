#include "HomeAssistantBridge.h"

#include "AppConfig.h"

HomeAssistantBridge *HomeAssistantBridge::_instance = nullptr;

namespace
{
HACover::CoverState toHaState(GateState state)
{
  switch (state)
  {
  case GateState::Open:
    return HACover::StateOpen;
  case GateState::Closed:
    return HACover::StateClosed;
  case GateState::Opening:
    return HACover::StateOpening;
  case GateState::Closing:
    return HACover::StateClosing;
  case GateState::Stopped:
    return HACover::StateStopped;
  case GateState::Unknown:
  default:
    return HACover::StateUnknown;
  }
}

int16_t toHaPosition(GateState state)
{
  if (state == GateState::Open)
  {
    return 100;
  }
  if (state == GateState::Closed)
  {
    return 0;
  }
  if (state == GateState::Unknown)
  {
    return HACover::DefaultPosition;
  }
  return 50;
}
}

HomeAssistantBridge::HomeAssistantBridge(GateController &gate,
                                         ClimateController &climate,
                                         NetworkManager &network,
                                         const char *deviceId)
    : _gate(gate),
      _climate(climate),
      _network(network),
      _device(deviceId),
      _mqtt(_client, _device),
      _cover("gate", HACover::PositionFeature),
      _wicketButton("wicket_open"),
      _wicketState("wicket_state"),
      _light("yard_light"),
      _temperature("temperature", HASensorNumber::PrecisionP1),
      _humidity("humidity", HASensorNumber::PrecisionP1)
{
  _instance = this;
}

void HomeAssistantBridge::begin(const char *mqttServer,
                                uint16_t mqttPort,
                                const char *mqttUser,
                                const char *mqttPassword)
{
  _client.setTimeout(1);

  _device.setName(DEVICE_NAME);
  _device.setManufacturer("Custom");
  _device.setModel("ESP32 roller gate controller");
  _device.setSoftwareVersion(SOFTWARE_VERSION);
  _device.enableExtendedUniqueIds();

  snprintf(_configurationUrl,
           sizeof(_configurationUrl),
           "http://%s/",
           _network.address().toString().c_str());
  _device.setConfigurationUrl(_configurationUrl);

  _cover.setName("Gate");
  _cover.setDeviceClass("garage");
  _cover.onCommand(onCoverCommand);

  _wicketButton.setName("Open wicket");
  _wicketButton.setIcon("mdi:gate");
  _wicketButton.onCommand(onWicketCommand);

  _wicketState.setName("Wicket state");
  _wicketState.setDeviceClass("door");

  _light.setName("Yard light");
  _light.setIcon("mdi:lightbulb");
  _light.onStateCommand(onLightCommand);

  _temperature.setName("Temperature");
  _temperature.setIcon("mdi:thermometer");
  _temperature.setUnitOfMeasurement("C");
  _temperature.setDeviceClass("temperature");
  _temperature.setStateClass("measurement");

  _humidity.setName("Humidity");
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

  const bool gateChanged = _lastGateRevision != _gate.stateRevision();
  const bool periodic = (millis() - _lastPublishAt) >= HA_PUBLISH_INTERVAL_MS;
  if (!gateChanged && !periodic)
  {
    return;
  }

  _lastGateRevision = _gate.stateRevision();
  _lastPublishAt = millis();
  publishAll(false);
}

bool HomeAssistantBridge::connected() const
{
  return _mqtt.isConnected();
}

void HomeAssistantBridge::publishAll(bool force)
{
  publishGate(force);
  _wicketState.setState(_gate.wicketOpen(), force);
  _light.setState(_gate.lightOn(), force);

  if (_climate.valid())
  {
    _temperature.setValue(_climate.temperature(), force);
    _humidity.setValue(_climate.humidity(), force);
  }
}

void HomeAssistantBridge::publishGate(bool force)
{
  const HACover::CoverState state = toHaState(_gate.state());
  if (state != HACover::StateUnknown)
  {
    _cover.setState(state, force);
  }

  const int16_t position = toHaPosition(_gate.state());
  if (position != HACover::DefaultPosition)
  {
    _cover.setPosition(position, force);
  }
}

void HomeAssistantBridge::onMqttConnected()
{
  if (_instance != nullptr)
  {
    _instance->publishAll(true);
  }
}

void HomeAssistantBridge::onCoverCommand(HACover::CoverCommand command, HACover *sender)
{
  (void)sender;
  if (_instance == nullptr)
  {
    return;
  }

  if (command == HACover::CommandOpen)
  {
    _instance->_gate.commandOpen();
  }
  else if (command == HACover::CommandClose)
  {
    _instance->_gate.commandClose();
  }
  else
  {
    _instance->_gate.commandStop();
  }
}

void HomeAssistantBridge::onWicketCommand(HAButton *sender)
{
  (void)sender;
  if (_instance != nullptr)
  {
    _instance->_gate.triggerWicket();
  }
}

void HomeAssistantBridge::onLightCommand(bool state, HALight *sender)
{
  (void)sender;
  if (_instance != nullptr)
  {
    _instance->_gate.setLight(state);
  }
}
