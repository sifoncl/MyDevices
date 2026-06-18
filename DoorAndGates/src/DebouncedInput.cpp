#include "DebouncedInput.h"

DebouncedInput::DebouncedInput(uint8_t pin, uint8_t activeLevel, unsigned long debounceMs)
    : _pin(pin),
      _activeLevel(activeLevel),
      _debounceMs(debounceMs)
{
}

void DebouncedInput::begin(uint8_t mode)
{
  pinMode(_pin, mode);
  const bool active = readActive();
  _lastRaw = active;
  _state = active;
  _lastChangeAt = millis();
  _pressStartedAt = active ? _lastChangeAt : 0;
}

void DebouncedInput::update()
{
  _justPressed = false;
  _justReleased = false;

  const bool raw = readActive();
  const unsigned long nowMs = millis();

  if (raw != _lastRaw)
  {
    _lastRaw = raw;
    _lastChangeAt = nowMs;
  }

  if ((nowMs - _lastChangeAt) >= _debounceMs && raw != _state)
  {
    _state = raw;
    _justPressed = _state;
    _justReleased = !_state;
    _pressStartedAt = _state ? nowMs : 0;
  }
}

bool DebouncedInput::isPressed() const
{
  return _state;
}

bool DebouncedInput::justPressed() const
{
  return _justPressed;
}

bool DebouncedInput::justReleased() const
{
  return _justReleased;
}

bool DebouncedInput::pressedFor(unsigned long ms) const
{
  return _state && (millis() - _pressStartedAt) >= ms;
}

bool DebouncedInput::readActive() const
{
  return digitalRead(_pin) == _activeLevel;
}
