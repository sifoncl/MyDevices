#ifndef RELAY_CONTROLLER_H
#define RELAY_CONTROLLER_H

#include <Arduino.h>

class RelayController
{
public:
  RelayController(const uint8_t *pins, uint8_t count, uint8_t activeLevel);

  void begin();
  void set(uint8_t index, bool enabled);
  void toggle(uint8_t index);
  bool state(uint8_t index) const;
  uint8_t count() const;
  uint32_t revision() const;

private:
  static constexpr uint8_t MAX_RELAYS = 8;

  const uint8_t *_pins;
  uint8_t _count;
  uint8_t _activeLevel;
  bool _states[MAX_RELAYS] = {};
  uint32_t _revision = 0;

  uint8_t inactiveLevel() const;
  void apply(uint8_t index);
};

#endif
