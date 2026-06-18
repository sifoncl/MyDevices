#include "ClimateReading.h"

const char *climateSourceName(ClimateSourceKind source)
{
  switch (source)
  {
  case ClimateSourceKind::HomeAssistant:
    return "HA";
  case ClimateSourceKind::Mqtt:
    return "MQTT";
  case ClimateSourceKind::EspNow:
    return "ESP-NOW";
  case ClimateSourceKind::None:
  default:
    return "none";
  }
}
