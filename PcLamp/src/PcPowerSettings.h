#ifndef PC_POWER_SETTINGS_H
#define PC_POWER_SETTINGS_H

#include <Arduino.h>
#include <Preferences.h>

struct PcPowerRuntimeSettings
{
  bool statusActiveHigh = true;
  bool buttonActiveHigh = true;
  uint32_t statusDebounceMs = 800;
  uint32_t buttonPulseMs = 250;
  uint32_t startRetryMs = 10000;
  uint32_t shutdownRetryMs = 12000;
  uint8_t maxStartAttempts = 2;
  uint8_t maxShutdownAttempts = 2;
};

class PcPowerSettings
{
public:
  explicit PcPowerSettings(const PcPowerRuntimeSettings &defaults);

  void begin();
  bool save(const PcPowerRuntimeSettings &settings);
  void restoreDefaults();

  const PcPowerRuntimeSettings &values() const;
  bool statusActiveHigh() const;
  bool buttonActiveHigh() const;
  uint32_t statusDebounceMs() const;
  uint32_t buttonPulseMs() const;
  uint32_t startRetryMs() const;
  uint32_t shutdownRetryMs() const;
  uint8_t maxStartAttempts() const;
  uint8_t maxShutdownAttempts() const;

private:
  PcPowerRuntimeSettings _defaults;
  PcPowerRuntimeSettings _values;
  Preferences _preferences;

  void loadDefaults();
  static PcPowerRuntimeSettings normalized(PcPowerRuntimeSettings settings);
};

#endif
