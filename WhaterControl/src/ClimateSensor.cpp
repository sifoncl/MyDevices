#include "ClimateSensor.h"

ClimateSensor::ClimateSensor(uint8_t pin, uint32_t readIntervalMs, uint32_t unavailableDelayMs)
    : _sensor(pin, DHT22),
      _readIntervalMs(readIntervalMs),
      _unavailableDelayMs(unavailableDelayMs)
{
}

void ClimateSensor::begin()
{
  _sensor.begin();
}

bool ClimateSensor::loop()
{
  const uint32_t now = millis();
  if ((now - _lastReadAt) < _readIntervalMs)
  {
    return false;
  }

  _lastReadAt = now;
  return readNow();
}

bool ClimateSensor::valid() const
{
  return _valid;
}

float ClimateSensor::temperature() const
{
  return _temperature;
}

float ClimateSensor::humidity() const
{
  return _humidity;
}

uint32_t ClimateSensor::revision() const
{
  return _revision;
}

const String &ClimateSensor::lastError() const
{
  return _lastError;
}

uint32_t ClimateSensor::lastSuccessAgeMs() const
{
  if (_lastSuccessAt == 0)
  {
    return UINT32_MAX;
  }
  return millis() - _lastSuccessAt;
}

bool ClimateSensor::readNow()
{
  const float newTemperature = _sensor.readTemperature();
  const float newHumidity = _sensor.readHumidity();

  if (valuesAreValid(newTemperature, newHumidity))
  {
    _temperature = newTemperature;
    _humidity = newHumidity;
    _lastSuccessAt = millis();
    _invalidTimerStarted = false;
    _invalidStartedAt = 0;
    _lastError = "";

    if (!_valid)
    {
      _valid = true;
    }
    _revision++;

    Serial.print("DHT temperature: ");
    Serial.print(_temperature);
    Serial.println(" C");
    Serial.print("DHT humidity: ");
    Serial.print(_humidity);
    Serial.println(" %");
    return true;
  }

  markInvalid("invalid DHT reading");
  return true;
}

bool ClimateSensor::valuesAreValid(float temperature, float humidity) const
{
  if (isnan(temperature) || isinf(temperature) || isnan(humidity) || isinf(humidity))
  {
    return false;
  }

  return temperature >= -40.0F && temperature < 100.0F &&
         humidity >= 0.0F && humidity < 105.0F;
}

void ClimateSensor::markInvalid(const String &error)
{
  const uint32_t now = millis();
  _lastError = error;

  if (!_invalidTimerStarted)
  {
    _invalidTimerStarted = true;
    _invalidStartedAt = now;
    Serial.println("DHT returned invalid data");
  }

  if (_valid && (now - _invalidStartedAt) >= _unavailableDelayMs)
  {
    _valid = false;
    _revision++;
    Serial.println("DHT sensor unavailable");
  }
}
