#include "ClimateSensor.h"

#include <math.h>

ClimateSensor::ClimateSensor(TwoWire &wire,
                             uint8_t address,
                             uint32_t readIntervalMs,
                             uint32_t retryIntervalMs)
    : _wire(wire),
      _address(address),
      _readIntervalMs(readIntervalMs),
      _retryIntervalMs(retryIntervalMs)
{
}

bool ClimateSensor::begin()
{
  return initializeSensor();
}

void ClimateSensor::loop()
{
  const uint32_t now = millis();

  if (!_initialized)
  {
    if ((now - _lastInitAttemptAt) >= _retryIntervalMs)
    {
      initializeSensor();
    }
    return;
  }

  if ((now - _lastReadAt) < _readIntervalMs)
  {
    return;
  }

  _lastReadAt = now;
  readMeasurement();
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

uint32_t ClimateSensor::lastSuccessAgeMs() const
{
  return _lastSuccessAt == 0 ? 0 : millis() - _lastSuccessAt;
}

const char *ClimateSensor::lastError() const
{
  return _lastError;
}

bool ClimateSensor::initializeSensor()
{
  _lastInitAttemptAt = millis();
  _sensor.begin(_wire, _address);
  _sensor.stopMeasurement();
  delay(2);

  int16_t error = _sensor.softReset();
  if (error != NO_ERROR)
  {
    setError(error, "soft reset");
    return false;
  }

  delay(100);
  uint16_t status = 0;
  error = _sensor.readStatusRegister(status);
  if (error != NO_ERROR)
  {
    setError(error, "status read");
    return false;
  }

  _initialized = true;
  _consecutiveErrors = 0;
  _lastReadAt = millis() - _readIntervalMs;
  snprintf(_lastError, sizeof(_lastError), "none");

  Serial.print("SHT85 ready, status: 0x");
  Serial.println(status, HEX);
  return true;
}

void ClimateSensor::readMeasurement()
{
  float temperature = 0.0f;
  float humidity = 0.0f;
  const int16_t error = _sensor.measureSingleShot(
      REPEATABILITY_MEDIUM,
      false,
      temperature,
      humidity);

  if (error != NO_ERROR || !isfinite(temperature) || !isfinite(humidity))
  {
    setError(error, "measurement");
    _consecutiveErrors++;
    if (_consecutiveErrors >= 3)
    {
      _initialized = false;
      _lastInitAttemptAt = millis();
    }
    return;
  }

  _temperature = temperature;
  _humidity = humidity;
  _valid = true;
  _consecutiveErrors = 0;
  _lastSuccessAt = millis();
  _revision++;
  snprintf(_lastError, sizeof(_lastError), "none");
}

void ClimateSensor::setError(int16_t errorCode, const char *operation)
{
  char detail[48] = "";
  if (errorCode != NO_ERROR)
  {
    errorToString(errorCode, detail, sizeof(detail));
  }
  else
  {
    snprintf(detail, sizeof(detail), "invalid value");
  }

  snprintf(_lastError, sizeof(_lastError), "%s: %s", operation, detail);
  _valid = false;
  _revision++;

  Serial.print("SHT85 error: ");
  Serial.println(_lastError);
}
