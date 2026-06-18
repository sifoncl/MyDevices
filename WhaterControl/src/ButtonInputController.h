#ifndef BUTTON_INPUT_CONTROLLER_H
#define BUTTON_INPUT_CONTROLLER_H

#include <Arduino.h>

#include "RelayController.h"

class ButtonInputController
{
public:
  ButtonInputController(const uint8_t *pins,
                        uint8_t count,
                        uint8_t activeLevel,
                        uint8_t inputMode,
                        uint32_t debounceMs,
                        RelayController &relays);

  void begin();
  void loop();

private:
  static constexpr uint8_t MAX_BUTTONS = 8;

  const uint8_t *_pins;
  uint8_t _count;
  uint8_t _activeLevel;
  uint8_t _inputMode;
  uint32_t _debounceMs;
  RelayController &_relays;

  bool _lastRaw[MAX_BUTTONS] = {};
  bool _stable[MAX_BUTTONS] = {};
  uint32_t _lastChangeAt[MAX_BUTTONS] = {};

  bool readActive(uint8_t index) const;
};

#endif
