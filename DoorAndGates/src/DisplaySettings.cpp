#include "DisplaySettings.h"

namespace
{
constexpr char SETTINGS_NAMESPACE[] = "display_cfg";
constexpr char KEY_BRIGHTNESS[] = "brightness";
}

DisplaySettings::DisplaySettings(uint16_t defaultBrightness,
                                 uint16_t minBrightness,
                                 uint16_t maxBrightness)
    : _defaultBrightness(defaultBrightness),
      _minBrightness(minBrightness),
      _maxBrightness(maxBrightness),
      _brightness(clamp(defaultBrightness))
{
}

void DisplaySettings::begin()
{
  _brightness = clamp(_defaultBrightness);
  if (_preferences.begin(SETTINGS_NAMESPACE, false))
  {
    _brightness = clamp(_preferences.getUShort(KEY_BRIGHTNESS, _brightness));
    _preferences.end();
  }
}

bool DisplaySettings::saveBrightness(uint16_t brightness)
{
  _brightness = clamp(brightness);
  if (!_preferences.begin(SETTINGS_NAMESPACE, false))
  {
    return false;
  }

  _preferences.putUShort(KEY_BRIGHTNESS, _brightness);
  _preferences.end();
  return true;
}

void DisplaySettings::restoreDefaults()
{
  if (_preferences.begin(SETTINGS_NAMESPACE, false))
  {
    _preferences.clear();
    _preferences.end();
  }
  _brightness = clamp(_defaultBrightness);
}

uint16_t DisplaySettings::brightness() const
{
  return _brightness;
}

uint16_t DisplaySettings::minBrightness() const
{
  return _minBrightness;
}

uint16_t DisplaySettings::maxBrightness() const
{
  return _maxBrightness;
}

uint16_t DisplaySettings::clamp(uint16_t value) const
{
  if (value < _minBrightness)
  {
    return _minBrightness;
  }
  if (value > _maxBrightness)
  {
    return _maxBrightness;
  }
  return value;
}
