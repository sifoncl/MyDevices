#ifndef CLIMATE_SENSOR_H
#define CLIMATE_SENSOR_H

#include <Arduino.h>
#include <SensirionI2cSht3x.h>
#include <Wire.h>

class ClimateSensor
{
public:
  ClimateSensor(TwoWire &wire,
                uint8_t address,
                uint32_t readIntervalMs,
                uint32_t retryIntervalMs);

  bool begin();
  void loop();

  bool valid() const;
  float temperature() const;
  float humidity() const;
  uint32_t revision() const;
  uint32_t lastSuccessAgeMs() const;
  const char *lastError() const;

private:
  TwoWire &_wire;
  SensirionI2cSht3x _sensor;
  uint8_t _address;
  uint32_t _readIntervalMs;
  uint32_t _retryIntervalMs;

  bool _initialized = false;
  bool _valid = false;
  float _temperature = 0.0f;
  float _humidity = 0.0f;
  uint32_t _revision = 0;
  uint32_t _lastReadAt = 0;
  uint32_t _lastInitAttemptAt = 0;
  uint32_t _lastSuccessAt = 0;
  uint8_t _consecutiveErrors = 0;
  char _lastError[80] = "not initialized";

  bool initializeSensor();
  void readMeasurement();
  void setError(int16_t errorCode, const char *operation);
};

#endif
