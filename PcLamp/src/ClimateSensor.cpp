#include "ClimateSensor.h"

#include <math.h>

ClimateSensor::ClimateSensor(uint8_t pin,
                             uint8_t type,
                             uint32_t readIntervalMs,
                             uint32_t invalidTimeoutMs)
    : _sensor(pin, type),
      _readIntervalMs(readIntervalMs),
      _invalidTimeoutMs(invalidTimeoutMs)
{
}

void ClimateSensor::begin()
{
  _sensor.begin();
  _lastReadAt = millis() - _readIntervalMs;
}

void ClimateSensor::loop()
{
  const uint32_t now = millis();
  if ((now - _lastReadAt) < _readIntervalMs)
  {
    return;
  }

  _lastReadAt = now;
  const float temperature = _sensor.readTemperature();
  const float humidity = _sensor.readHumidity();

  if (readingLooksValid(temperature, humidity))
  {
    const bool changed = !_valid ||
                         fabsf(temperature - _temperature) >= 0.05f ||
                         fabsf(humidity - _humidity) >= 0.05f;

    _temperature = temperature;
    _humidity = humidity;
    _valid = true;
    _invalidPeriodActive = false;
    _lastSuccessAt = now;
    _lastError = "none";

    if (changed)
    {
      _revision++;
    }

    Serial.print("DHT temperature: ");
    Serial.print(_temperature);
    Serial.print(" C, humidity: ");
    Serial.print(_humidity);
    Serial.println(" %");
    return;
  }

  setInvalid(isnan(temperature) || isnan(humidity) ? "nan" : "out_of_range", now);
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

uint32_t ClimateSensor::lastReadAgeMs() const
{
  return millis() - _lastReadAt;
}

uint32_t ClimateSensor::lastSuccessAgeMs() const
{
  return _lastSuccessAt == 0 ? UINT32_MAX : millis() - _lastSuccessAt;
}

uint32_t ClimateSensor::revision() const
{
  return _revision;
}

const String &ClimateSensor::lastError() const
{
  return _lastError;
}

void ClimateSensor::setInvalid(const String &error, uint32_t now)
{
  _lastError = error;

  if (!_invalidPeriodActive)
  {
    _invalidPeriodActive = true;
    _invalidStartedAt = now;
    Serial.print("DHT invalid reading: ");
    Serial.println(error);
  }

  if (_valid && (now - _invalidStartedAt) >= _invalidTimeoutMs)
  {
    _valid = false;
    _revision++;
    Serial.println("DHT marked unavailable");
  }
}

bool ClimateSensor::readingLooksValid(float temperature, float humidity)
{
  return !isnan(temperature) &&
         !isnan(humidity) &&
         temperature > -40.0f &&
         temperature < 100.0f &&
         humidity >= 0.0f &&
         humidity <= 105.0f;
}
