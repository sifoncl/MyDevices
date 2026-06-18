#ifndef TEMPERATURE_BOOST_SETTINGS_H
#define TEMPERATURE_BOOST_SETTINGS_H

#include <Arduino.h>
#include <Preferences.h>

class TemperatureBoostSettings
{
public:
  TemperatureBoostSettings(bool defaultEnabled,
                           float defaultFactor,
                           float minFactor,
                           float maxFactor,
                           float baseline);

  void begin();
  bool save(bool enabled, float factor);
  void restoreDefaults();

  bool enabled() const;
  float factor() const;
  float minFactor() const;
  float maxFactor() const;
  float baseline() const;
  float apply(float temperature) const;

private:
  bool _defaultEnabled;
  float _defaultFactor;
  float _minFactor;
  float _maxFactor;
  float _baseline;

  bool _enabled = false;
  float _factor = 0.0f;
  Preferences _preferences;

  float clampFactor(float factor) const;
  void loadDefaults();
};

#endif
