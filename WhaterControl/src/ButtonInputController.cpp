#include "ButtonInputController.h"

ButtonInputController::ButtonInputController(const uint8_t *pins,
                                             uint8_t count,
                                             uint8_t activeLevel,
                                             uint8_t inputMode,
                                             uint32_t debounceMs,
                                             RelayController &relays)
    : _pins(pins),
      _count(count > MAX_BUTTONS ? MAX_BUTTONS : count),
      _activeLevel(activeLevel),
      _inputMode(inputMode),
      _debounceMs(debounceMs),
      _relays(relays)
{
}

void ButtonInputController::begin()
{
  const uint32_t now = millis();
  for (uint8_t i = 0; i < _count; i++)
  {
    pinMode(_pins[i], _inputMode);
    const bool active = readActive(i);
    _lastRaw[i] = active;
    _stable[i] = active;
    _lastChangeAt[i] = now;
  }
}

void ButtonInputController::loop()
{
  const uint32_t now = millis();
  for (uint8_t i = 0; i < _count; i++)
  {
    const bool raw = readActive(i);
    if (raw != _lastRaw[i])
    {
      _lastRaw[i] = raw;
      _lastChangeAt[i] = now;
    }

    if ((now - _lastChangeAt[i]) < _debounceMs || raw == _stable[i])
    {
      continue;
    }

    _stable[i] = raw;
    if (_stable[i])
    {
      _relays.toggle(i);
      Serial.print("Button ");
      Serial.print(i + 1);
      Serial.println(" pressed");
    }
  }
}

bool ButtonInputController::readActive(uint8_t index) const
{
  return digitalRead(_pins[index]) == _activeLevel;
}
