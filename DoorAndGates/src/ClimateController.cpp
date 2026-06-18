#include "ClimateController.h"

ClimateController::ClimateController(uint8_t address, unsigned long intervalMs)
    : _address(address),
      _intervalMs(intervalMs)
{
}

void ClimateController::begin()
{
  _available = _sensor.begin(_address);
  _lastReadAt = millis() - _intervalMs;
  Serial.println(_available ? "SHT31 ready" : "SHT31 not found");
  loop();
}

bool ClimateController::loop()
{
  if ((millis() - _lastReadAt) < _intervalMs)
  {
    return false;
  }
  _lastReadAt = millis();

  if (!_available)
  {
    return false;
  }

  const float newTemperature = _sensor.readTemperature();
  const float newHumidity = _sensor.readHumidity();
  if (isnan(newTemperature) || isnan(newHumidity))
  {
    return false;
  }

  _temperature = newTemperature;
  _humidity = newHumidity;
  return true;
}

bool ClimateController::available() const
{
  return _available;
}

bool ClimateController::valid() const
{
  return _available && !isnan(_temperature) && !isnan(_humidity);
}

float ClimateController::temperature() const
{
  return _temperature;
}

float ClimateController::humidity() const
{
  return _humidity;
}
