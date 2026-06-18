#include "MagnetAxisTracker.h"

#include <math.h>

#if defined(ARDUINO_ARCH_ESP32)
#include <Preferences.h>
#endif

MagnetAxisTracker::MagnetAxisTracker(const MagnetAxisTrackerConfig &config)
    : _config(config)
{
}

MagnetAxisTrackerState MagnetAxisTracker::update(float x, float y, float z)
{
  const float raw = selectValue(x, y, z);

  if (!_hasSample)
  {
    _filtered = raw;
    _hasSample = true;
  }
  else
  {
    const float samples = max<uint8_t>(1, _config.smoothingSamples);
    const float alpha = 1.0f / samples;
    _filtered += (raw - _filtered) * alpha;
  }

  if (_config.autoExpandCalibration)
  {
    applySampleToCalibration(_filtered);
  }

  _state.raw = raw;
  _state.filtered = _filtered;
  updateOutputState();

  return _state;
}

void MagnetAxisTracker::resetCalibration()
{
  _hasAnyCalibrationValue = false;
  _hasFirstLimit = false;
  _hasSecondLimit = false;
  _minValue = 0.0f;
  _maxValue = 0.0f;
  _active = false;
  updateOutputState();
}

#if defined(ARDUINO_ARCH_ESP32)
bool MagnetAxisTracker::loadCalibration(const char *namespaceName)
{
  if (namespaceName == nullptr)
  {
    return false;
  }

  Preferences preferences;
  if (!preferences.begin(namespaceName, true))
  {
    return false;
  }

  const bool hasStoredCalibration = preferences.isKey("min") && preferences.isKey("max");
  if (!hasStoredCalibration)
  {
    preferences.end();
    return false;
  }

  const float storedMin = preferences.getFloat("min", 0.0f);
  const float storedMax = preferences.getFloat("max", 0.0f);
  preferences.end();

  _minValue = min(storedMin, storedMax);
  _maxValue = max(storedMin, storedMax);
  _firstLimit = _minValue;
  _secondLimit = _maxValue;
  _hasAnyCalibrationValue = true;
  _hasFirstLimit = true;
  _hasSecondLimit = true;
  updateOutputState();

  return hasCalibration();
}

bool MagnetAxisTracker::saveCalibration(const char *namespaceName) const
{
  if (namespaceName == nullptr || !hasCalibration())
  {
    return false;
  }

  Preferences preferences;
  if (!preferences.begin(namespaceName, false))
  {
    return false;
  }

  const bool savedMin = preferences.putFloat("min", _minValue) > 0;
  const bool savedMax = preferences.putFloat("max", _maxValue) > 0;
  preferences.end();

  return savedMin && savedMax;
}

bool MagnetAxisTracker::clearStoredCalibration(const char *namespaceName) const
{
  if (namespaceName == nullptr)
  {
    return false;
  }

  Preferences preferences;
  if (!preferences.begin(namespaceName, false))
  {
    return false;
  }

  const bool cleared = preferences.clear();
  preferences.end();
  return cleared;
}
#endif

void MagnetAxisTracker::setCalibration(float firstLimitValue, float secondLimitValue)
{
  _firstLimit = firstLimitValue;
  _secondLimit = secondLimitValue;
  _hasFirstLimit = true;
  _hasSecondLimit = true;
  rebuildCalibrationFromLimits();
  updateOutputState();
}

bool MagnetAxisTracker::rememberCurrentAsFirstLimit()
{
  if (!_hasSample)
  {
    return false;
  }

  _firstLimit = _filtered;
  _hasFirstLimit = true;
  rebuildCalibrationFromLimits();
  updateOutputState();
  return true;
}

bool MagnetAxisTracker::rememberCurrentAsSecondLimit()
{
  if (!_hasSample)
  {
    return false;
  }

  _secondLimit = _filtered;
  _hasSecondLimit = true;
  rebuildCalibrationFromLimits();
  updateOutputState();
  return true;
}

void MagnetAxisTracker::rememberFirstLimit(float x, float y, float z)
{
  _firstLimit = selectValue(x, y, z);
  _hasFirstLimit = true;
  rebuildCalibrationFromLimits();
  updateOutputState();
}

void MagnetAxisTracker::rememberSecondLimit(float x, float y, float z)
{
  _secondLimit = selectValue(x, y, z);
  _hasSecondLimit = true;
  rebuildCalibrationFromLimits();
  updateOutputState();
}

bool MagnetAxisTracker::hasCalibration() const
{
  return _hasAnyCalibrationValue && fabsf(_maxValue - _minValue) >= _config.minCalibratedSpan;
}

bool MagnetAxisTracker::isActive() const
{
  return _state.active;
}

bool MagnetAxisTracker::justActivated() const
{
  return _state.justActivated;
}

bool MagnetAxisTracker::justDeactivated() const
{
  return _state.justDeactivated;
}

float MagnetAxisTracker::positionPercent() const
{
  return _state.positionPercent;
}

float MagnetAxisTracker::filteredValue() const
{
  return _state.filtered;
}

const MagnetAxisTrackerState &MagnetAxisTracker::state() const
{
  return _state;
}

float MagnetAxisTracker::selectValue(float x, float y, float z) const
{
  float value = 0.0f;

  switch (_config.axis)
  {
  case MagnetAxis::X:
    value = x;
    break;
  case MagnetAxis::Y:
    value = y;
    break;
  case MagnetAxis::Z:
    value = z;
    break;
  case MagnetAxis::Magnitude:
    value = sqrtf((x * x) + (y * y) + (z * z));
    break;
  }

  if (_config.useAbsoluteValue && _config.axis != MagnetAxis::Magnitude)
  {
    value = fabsf(value);
  }

  return value;
}

void MagnetAxisTracker::applySampleToCalibration(float value)
{
  if (!_hasAnyCalibrationValue)
  {
    _minValue = value;
    _maxValue = value;
    _hasAnyCalibrationValue = true;
    return;
  }

  if (value < _minValue)
  {
    _minValue = value;
  }
  if (value > _maxValue)
  {
    _maxValue = value;
  }
}

void MagnetAxisTracker::rebuildCalibrationFromLimits()
{
  if (_hasFirstLimit && _hasSecondLimit)
  {
    _minValue = min(_firstLimit, _secondLimit);
    _maxValue = max(_firstLimit, _secondLimit);
    _hasAnyCalibrationValue = true;
    return;
  }

  if (_hasFirstLimit)
  {
    _minValue = _firstLimit;
    _maxValue = _firstLimit;
    _hasAnyCalibrationValue = true;
    return;
  }

  if (_hasSecondLimit)
  {
    _minValue = _secondLimit;
    _maxValue = _secondLimit;
    _hasAnyCalibrationValue = true;
  }
}

void MagnetAxisTracker::updateOutputState()
{
  _state.minValue = _minValue;
  _state.maxValue = _maxValue;
  _state.calibrated = hasCalibration();
  _state.justActivated = false;
  _state.justDeactivated = false;

  if (!_state.calibrated)
  {
    _state.positionPercent = 0.0f;
    _state.targetDistancePercent = 100.0f;
    _state.active = false;
    _active = false;
    return;
  }

  const float span = _maxValue - _minValue;
  float position = ((_filtered - _minValue) * 100.0f) / span;
  position = constrain(position, 0.0f, 100.0f);

  const float distanceToTarget =
      (_config.activeTarget == MagnetActiveTarget::HighEnd) ? (100.0f - position) : position;

  const float activeWindow = constrain(_config.activeWindowPercent, 0.0f, 100.0f);
  const float releaseWindow = constrain(activeWindow + max(0.0f, _config.hysteresisPercent), 0.0f, 100.0f);

  bool nextActive = _active;

  if (_active)
  {
    nextActive = distanceToTarget <= releaseWindow;
  }
  else
  {
    nextActive = distanceToTarget <= activeWindow;
  }

  _state.positionPercent = position;
  _state.targetDistancePercent = distanceToTarget;
  _state.active = nextActive;
  _state.justActivated = !_active && nextActive;
  _state.justDeactivated = _active && !nextActive;
  _active = nextActive;
}
