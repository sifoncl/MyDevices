#ifndef DEBOUNCED_INPUT_H
#define DEBOUNCED_INPUT_H

#include <Arduino.h>

class DebouncedInput
{
public:
  DebouncedInput(uint8_t pin, uint8_t activeLevel, unsigned long debounceMs);

  void begin(uint8_t mode);
  void update();

  bool isPressed() const;
  bool justPressed() const;
  bool justReleased() const;
  bool pressedFor(unsigned long ms) const;

private:
  uint8_t _pin;
  uint8_t _activeLevel;
  unsigned long _debounceMs;
  bool _lastRaw = false;
  bool _state = false;
  bool _justPressed = false;
  bool _justReleased = false;
  unsigned long _lastChangeAt = 0;
  unsigned long _pressStartedAt = 0;

  bool readActive() const;
};

#endif
