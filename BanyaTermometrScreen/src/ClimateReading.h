#ifndef CLIMATE_READING_H
#define CLIMATE_READING_H

#include <Arduino.h>

enum class ClimateSourceKind : uint8_t
{
  None = 0,
  HomeAssistant,
  Mqtt,
  EspNow
};

struct ClimateReading
{
  bool valid = false;
  float rawTemperature = 0.0f;
  float displayTemperature = 0.0f;
  float humidity = 0.0f;
  ClimateSourceKind source = ClimateSourceKind::None;
  uint32_t ageMs = 0;
  uint32_t revision = 0;

  float temperature() const
  {
    return displayTemperature;
  }
};

const char *climateSourceName(ClimateSourceKind source);

#endif
