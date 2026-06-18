#ifndef CLIMATE_SENSOR_H
#define CLIMATE_SENSOR_H

#include <Arduino.h>
#include <DHT.h>

class ClimateSensor
{
public:
  ClimateSensor(uint8_t pin,
                uint8_t type,
                uint32_t readIntervalMs,
                uint32_t invalidTimeoutMs);

  void begin();
  void loop();

  bool valid() const;
  float temperature() const;
  float humidity() const;
  uint32_t lastReadAgeMs() const;
  uint32_t lastSuccessAgeMs() const;
  uint32_t revision() const;
  const String &lastError() const;

private:
  DHT _sensor;
  uint32_t _readIntervalMs;
  uint32_t _invalidTimeoutMs;

  float _temperature = NAN;
  float _humidity = NAN;
  bool _valid = false;
  bool _invalidPeriodActive = false;
  uint32_t _lastReadAt = 0;
  uint32_t _lastSuccessAt = 0;
  uint32_t _invalidStartedAt = 0;
  uint32_t _revision = 0;
  String _lastError = "not_read";

  void setInvalid(const String &error, uint32_t now);
  static bool readingLooksValid(float temperature, float humidity);
};

#endif
