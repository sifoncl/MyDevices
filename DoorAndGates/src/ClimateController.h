#ifndef CLIMATE_CONTROLLER_H
#define CLIMATE_CONTROLLER_H

#include <Adafruit_SHT31.h>

class ClimateController
{
public:
  ClimateController(uint8_t address, unsigned long intervalMs);

  void begin();
  bool loop();

  bool available() const;
  bool valid() const;
  float temperature() const;
  float humidity() const;

private:
  uint8_t _address;
  unsigned long _intervalMs;
  unsigned long _lastReadAt = 0;
  Adafruit_SHT31 _sensor;
  bool _available = false;
  float _temperature = NAN;
  float _humidity = NAN;
};

#endif
