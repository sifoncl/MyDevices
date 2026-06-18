#include "PcPowerSettings.h"

namespace
{
constexpr char SETTINGS_NAMESPACE[] = "pc_power";
constexpr char KEY_STATUS_HIGH[] = "status_high";
constexpr char KEY_BUTTON_HIGH[] = "button_high";
constexpr char KEY_DEBOUNCE_MS[] = "debounce";
constexpr char KEY_PULSE_MS[] = "pulse";
constexpr char KEY_START_RETRY_MS[] = "start_retry";
constexpr char KEY_STOP_RETRY_MS[] = "stop_retry";
constexpr char KEY_START_ATTEMPTS[] = "start_tries";
constexpr char KEY_STOP_ATTEMPTS[] = "stop_tries";

uint32_t clampU32(uint32_t value, uint32_t minValue, uint32_t maxValue)
{
  if (value < minValue)
  {
    return minValue;
  }
  if (value > maxValue)
  {
    return maxValue;
  }
  return value;
}

uint8_t clampU8(uint8_t value, uint8_t minValue, uint8_t maxValue)
{
  if (value < minValue)
  {
    return minValue;
  }
  if (value > maxValue)
  {
    return maxValue;
  }
  return value;
}
}

PcPowerSettings::PcPowerSettings(const PcPowerRuntimeSettings &defaults)
    : _defaults(normalized(defaults)),
      _values(_defaults)
{
}

void PcPowerSettings::begin()
{
  loadDefaults();

  if (_preferences.begin(SETTINGS_NAMESPACE, false))
  {
    _values.statusActiveHigh = _preferences.getBool(KEY_STATUS_HIGH, _values.statusActiveHigh);
    _values.buttonActiveHigh = _preferences.getBool(KEY_BUTTON_HIGH, _values.buttonActiveHigh);
    _values.statusDebounceMs = _preferences.getUInt(KEY_DEBOUNCE_MS, _values.statusDebounceMs);
    _values.buttonPulseMs = _preferences.getUInt(KEY_PULSE_MS, _values.buttonPulseMs);
    _values.startRetryMs = _preferences.getUInt(KEY_START_RETRY_MS, _values.startRetryMs);
    _values.shutdownRetryMs = _preferences.getUInt(KEY_STOP_RETRY_MS, _values.shutdownRetryMs);
    _values.maxStartAttempts = _preferences.getUChar(KEY_START_ATTEMPTS, _values.maxStartAttempts);
    _values.maxShutdownAttempts = _preferences.getUChar(KEY_STOP_ATTEMPTS, _values.maxShutdownAttempts);
    _preferences.end();
  }

  _values = normalized(_values);
}

bool PcPowerSettings::save(const PcPowerRuntimeSettings &settings)
{
  const PcPowerRuntimeSettings next = normalized(settings);
  if (!_preferences.begin(SETTINGS_NAMESPACE, false))
  {
    return false;
  }

  _preferences.putBool(KEY_STATUS_HIGH, next.statusActiveHigh);
  _preferences.putBool(KEY_BUTTON_HIGH, next.buttonActiveHigh);
  _preferences.putUInt(KEY_DEBOUNCE_MS, next.statusDebounceMs);
  _preferences.putUInt(KEY_PULSE_MS, next.buttonPulseMs);
  _preferences.putUInt(KEY_START_RETRY_MS, next.startRetryMs);
  _preferences.putUInt(KEY_STOP_RETRY_MS, next.shutdownRetryMs);
  _preferences.putUChar(KEY_START_ATTEMPTS, next.maxStartAttempts);
  _preferences.putUChar(KEY_STOP_ATTEMPTS, next.maxShutdownAttempts);
  _preferences.end();

  _values = next;
  return true;
}

void PcPowerSettings::restoreDefaults()
{
  if (_preferences.begin(SETTINGS_NAMESPACE, false))
  {
    _preferences.clear();
    _preferences.end();
  }
  loadDefaults();
}

const PcPowerRuntimeSettings &PcPowerSettings::values() const
{
  return _values;
}

bool PcPowerSettings::statusActiveHigh() const
{
  return _values.statusActiveHigh;
}

bool PcPowerSettings::buttonActiveHigh() const
{
  return _values.buttonActiveHigh;
}

uint32_t PcPowerSettings::statusDebounceMs() const
{
  return _values.statusDebounceMs;
}

uint32_t PcPowerSettings::buttonPulseMs() const
{
  return _values.buttonPulseMs;
}

uint32_t PcPowerSettings::startRetryMs() const
{
  return _values.startRetryMs;
}

uint32_t PcPowerSettings::shutdownRetryMs() const
{
  return _values.shutdownRetryMs;
}

uint8_t PcPowerSettings::maxStartAttempts() const
{
  return _values.maxStartAttempts;
}

uint8_t PcPowerSettings::maxShutdownAttempts() const
{
  return _values.maxShutdownAttempts;
}

void PcPowerSettings::loadDefaults()
{
  _values = _defaults;
}

PcPowerRuntimeSettings PcPowerSettings::normalized(PcPowerRuntimeSettings settings)
{
  settings.statusDebounceMs = clampU32(settings.statusDebounceMs, 20UL, 10000UL);
  settings.buttonPulseMs = clampU32(settings.buttonPulseMs, 50UL, 5000UL);
  settings.startRetryMs = clampU32(settings.startRetryMs, 1000UL, 120000UL);
  settings.shutdownRetryMs = clampU32(settings.shutdownRetryMs, 1000UL, 120000UL);
  settings.maxStartAttempts = clampU8(settings.maxStartAttempts, 1, 10);
  settings.maxShutdownAttempts = clampU8(settings.maxShutdownAttempts, 1, 10);
  return settings;
}
