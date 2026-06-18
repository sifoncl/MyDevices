#ifndef ESP_NOW_CLIMATE_BROADCASTER_H
#define ESP_NOW_CLIMATE_BROADCASTER_H

#include <Arduino.h>

#include "ClimateSensor.h"

class EspNowClimateBroadcaster
{
public:
  explicit EspNowClimateBroadcaster(uint32_t publishIntervalMs);

  void begin();
  void loop(const ClimateSensor &climate);
  bool ready() const;

private:
  uint32_t _publishIntervalMs;
  uint32_t _lastPublishAt = 0;
  uint32_t _lastRevision = UINT32_MAX;
  bool _ready = false;

  void publish(const ClimateSensor &climate);
};

#endif
