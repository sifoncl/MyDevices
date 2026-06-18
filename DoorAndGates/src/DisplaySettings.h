#ifndef DISPLAY_SETTINGS_H
#define DISPLAY_SETTINGS_H

#include <Arduino.h>
#include <Preferences.h>

class DisplaySettings
{
public:
  DisplaySettings(uint16_t defaultBrightness,
                  uint16_t minBrightness,
                  uint16_t maxBrightness);

  void begin();
  bool saveBrightness(uint16_t brightness);
  void restoreDefaults();

  uint16_t brightness() const;
  uint16_t minBrightness() const;
  uint16_t maxBrightness() const;

private:
  uint16_t _defaultBrightness;
  uint16_t _minBrightness;
  uint16_t _maxBrightness;
  uint16_t _brightness;
  Preferences _preferences;

  uint16_t clamp(uint16_t value) const;
};

#endif
