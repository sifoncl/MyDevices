#ifndef DISPLAY_SETTINGS_H
#define DISPLAY_SETTINGS_H

#include <Arduino.h>
#include <Preferences.h>

class DisplaySettings
{
public:
  DisplaySettings(uint16_t defaultBrightness,
                  uint16_t minBrightness,
                  uint16_t maxBrightness,
                  bool defaultEnabled = true);

  void begin();
  bool saveBrightness(uint16_t brightness);
  bool saveEnabled(bool enabled);
  bool save(uint16_t brightness, bool enabled);
  void restoreDefaults();

  uint16_t brightness() const;
  bool enabled() const;
  uint16_t minBrightness() const;
  uint16_t maxBrightness() const;

private:
  uint16_t _defaultBrightness;
  uint16_t _minBrightness;
  uint16_t _maxBrightness;
  bool _defaultEnabled;
  uint16_t _brightness;
  bool _enabled;
  Preferences _preferences;

  uint16_t clamp(uint16_t value) const;
};

#endif
