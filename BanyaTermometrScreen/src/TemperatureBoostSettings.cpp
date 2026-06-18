#include "TemperatureBoostSettings.h"

namespace
{
constexpr char SETTINGS_NAMESPACE[] = "boost_cfg";
constexpr char KEY_ENABLED[] = "enabled";
constexpr char KEY_FACTOR[] = "factor";
}

TemperatureBoostSettings::TemperatureBoostSettings(bool defaultEnabled,
                                                   float defaultFactor,
                                                   float minFactor,
                                                   float maxFactor,
                                                   float baseline)
    : _defaultEnabled(defaultEnabled),
      _defaultFactor(defaultFactor),
      _minFactor(minFactor),
      _maxFactor(maxFactor),
      _baseline(baseline)
{
  loadDefaults();
}

void TemperatureBoostSettings::begin()
{
  loadDefaults();
  if (_preferences.begin(SETTINGS_NAMESPACE, false))
  {
    _enabled = _preferences.getBool(KEY_ENABLED, _enabled);
    _factor = clampFactor(_preferences.getFloat(KEY_FACTOR, _factor));
    _preferences.end();
  }
}

bool TemperatureBoostSettings::save(bool enabled, float factor)
{
  _enabled = enabled;
  _factor = clampFactor(factor);

  if (!_preferences.begin(SETTINGS_NAMESPACE, false))
  {
    return false;
  }

  _preferences.putBool(KEY_ENABLED, _enabled);
  _preferences.putFloat(KEY_FACTOR, _factor);
  _preferences.end();
  return true;
}

void TemperatureBoostSettings::restoreDefaults()
{
  if (_preferences.begin(SETTINGS_NAMESPACE, false))
  {
    _preferences.clear();
    _preferences.end();
  }
  loadDefaults();
}

bool TemperatureBoostSettings::enabled() const
{
  return _enabled;
}

float TemperatureBoostSettings::factor() const
{
  return _factor;
}

float TemperatureBoostSettings::minFactor() const
{
  return _minFactor;
}

float TemperatureBoostSettings::maxFactor() const
{
  return _maxFactor;
}

float TemperatureBoostSettings::baseline() const
{
  return _baseline;
}

float TemperatureBoostSettings::apply(float temperature) const
{
  if (!_enabled || temperature <= _baseline)
  {
    return temperature;
  }

  return _baseline + ((temperature - _baseline) * (1.0f + _factor));
}

float TemperatureBoostSettings::clampFactor(float factor) const
{
  if (factor < _minFactor)
  {
    return _minFactor;
  }
  if (factor > _maxFactor)
  {
    return _maxFactor;
  }
  return factor;
}

void TemperatureBoostSettings::loadDefaults()
{
  _enabled = _defaultEnabled;
  _factor = clampFactor(_defaultFactor);
}
