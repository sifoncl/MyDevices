#ifndef PRESSURE_SENSOR_H
#define PRESSURE_SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BME280.h>

class PressureSensor
{
public:
  PressureSensor(TwoWire &wire,
                 uint8_t address,
                 bool enabled,
                 uint32_t readIntervalMs,
                 uint32_t unavailableDelayMs);

  void begin();
  bool loop();

  bool enabled() const;
  bool detected() const;
  bool valid() const;
  float pressureMmHg() const;
  uint32_t revision() const;
  const String &lastError() const;
  uint32_t lastSuccessAgeMs() const;

private:
  TwoWire &_wire;
  Adafruit_BME280 _sensor;
  uint8_t _address;
  bool _enabled;
  uint32_t _readIntervalMs;
  uint32_t _unavailableDelayMs;

  uint32_t _lastReadAt = 0;
  uint32_t _invalidStartedAt = 0;
  uint32_t _lastSuccessAt = 0;
  bool _detected = false;
  bool _invalidTimerStarted = false;
  bool _valid = false;
  float _pressureMmHg = NAN;
  uint32_t _revision = 0;
  String _lastError;

  bool readNow();
  bool valueIsValid(float pressureMmHg) const;
  void markInvalid(const String &error);
};

#endif
