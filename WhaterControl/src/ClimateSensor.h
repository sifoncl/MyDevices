#ifndef CLIMATE_SENSOR_H
#define CLIMATE_SENSOR_H

#include <Arduino.h>
#include <DHT.h>

class ClimateSensor
{
public:
  ClimateSensor(uint8_t pin, uint32_t readIntervalMs, uint32_t unavailableDelayMs);

  void begin();
  bool loop();

  bool valid() const;
  float temperature() const;
  float humidity() const;
  uint32_t revision() const;
  const String &lastError() const;
  uint32_t lastSuccessAgeMs() const;

private:
  DHT _sensor;
  uint32_t _readIntervalMs;
  uint32_t _unavailableDelayMs;

  uint32_t _lastReadAt = 0;
  uint32_t _invalidStartedAt = 0;
  uint32_t _lastSuccessAt = 0;
  bool _invalidTimerStarted = false;
  bool _valid = false;
  float _temperature = NAN;
  float _humidity = NAN;
  uint32_t _revision = 0;
  String _lastError;

  bool readNow();
  bool valuesAreValid(float temperature, float humidity) const;
  void markInvalid(const String &error);
};

#endif
