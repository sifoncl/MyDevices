#include "PressureSensor.h"

PressureSensor::PressureSensor(TwoWire &wire,
                               uint8_t address,
                               bool enabled,
                               uint32_t readIntervalMs,
                               uint32_t unavailableDelayMs)
    : _wire(wire),
      _address(address),
      _enabled(enabled),
      _readIntervalMs(readIntervalMs),
      _unavailableDelayMs(unavailableDelayMs)
{
}

void PressureSensor::begin()
{
  if (!_enabled)
  {
    _lastError = "pressure sensor disabled";
    return;
  }

  _detected = _sensor.begin(_address, &_wire);
  if (!_detected)
  {
    _lastError = "BME280 not found";
    Serial.println("BME280 not found; pressure sensor will stay unavailable");
    return;
  }

  Serial.println("BME280 found");
}

bool PressureSensor::loop()
{
  if (!_enabled || !_detected)
  {
    return false;
  }

  const uint32_t now = millis();
  if ((now - _lastReadAt) < _readIntervalMs)
  {
    return false;
  }

  _lastReadAt = now;
  return readNow();
}

bool PressureSensor::enabled() const
{
  return _enabled;
}

bool PressureSensor::detected() const
{
  return _detected;
}

bool PressureSensor::valid() const
{
  return _valid;
}

float PressureSensor::pressureMmHg() const
{
  return _pressureMmHg;
}

uint32_t PressureSensor::revision() const
{
  return _revision;
}

const String &PressureSensor::lastError() const
{
  return _lastError;
}

uint32_t PressureSensor::lastSuccessAgeMs() const
{
  if (_lastSuccessAt == 0)
  {
    return UINT32_MAX;
  }
  return millis() - _lastSuccessAt;
}

bool PressureSensor::readNow()
{
  const float pressureMmHg = _sensor.readPressure() / 133.322368F;

  if (valueIsValid(pressureMmHg))
  {
    _pressureMmHg = pressureMmHg;
    _lastSuccessAt = millis();
    _invalidTimerStarted = false;
    _invalidStartedAt = 0;
    _lastError = "";

    if (!_valid)
    {
      _valid = true;
    }
    _revision++;

    Serial.print("BME280 pressure: ");
    Serial.print(_pressureMmHg);
    Serial.println(" mmHg");
    return true;
  }

  markInvalid("invalid BME280 pressure reading");
  return true;
}

bool PressureSensor::valueIsValid(float pressureMmHg) const
{
  if (isnan(pressureMmHg) || isinf(pressureMmHg))
  {
    return false;
  }

  return pressureMmHg >= 300.0F && pressureMmHg <= 900.0F;
}

void PressureSensor::markInvalid(const String &error)
{
  const uint32_t now = millis();
  _lastError = error;

  if (!_invalidTimerStarted)
  {
    _invalidTimerStarted = true;
    _invalidStartedAt = now;
    Serial.println("BME280 returned invalid pressure data");
  }

  if (_valid && (now - _invalidStartedAt) >= _unavailableDelayMs)
  {
    _valid = false;
    _revision++;
    Serial.println("BME280 pressure unavailable");
  }
}
