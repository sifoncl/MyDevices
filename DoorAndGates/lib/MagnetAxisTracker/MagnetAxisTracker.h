#ifndef MAGNET_AXIS_TRACKER_H
#define MAGNET_AXIS_TRACKER_H

#include <Arduino.h>

enum class MagnetAxis : uint8_t
{
  X,
  Y,
  Z,
  Magnitude
};

enum class MagnetActiveTarget : uint8_t
{
  LowEnd,
  HighEnd
};

struct MagnetAxisTrackerConfig
{
  MagnetAxis axis = MagnetAxis::X;
  MagnetActiveTarget activeTarget = MagnetActiveTarget::HighEnd;
  bool useAbsoluteValue = true;
  bool autoExpandCalibration = true;
  uint8_t smoothingSamples = 6;
  float minCalibratedSpan = 5.0f;
  float activeWindowPercent = 15.0f;
  float hysteresisPercent = 5.0f;
};

struct MagnetAxisTrackerState
{
  float raw = 0.0f;
  float filtered = 0.0f;
  float minValue = 0.0f;
  float maxValue = 0.0f;
  float positionPercent = 0.0f;
  float targetDistancePercent = 100.0f;
  bool calibrated = false;
  bool active = false;
  bool justActivated = false;
  bool justDeactivated = false;
};

class MagnetAxisTracker
{
public:
  explicit MagnetAxisTracker(const MagnetAxisTrackerConfig &config = MagnetAxisTrackerConfig());

  MagnetAxisTrackerState update(float x, float y, float z);

  void resetCalibration();
  void setCalibration(float firstLimitValue, float secondLimitValue);

#if defined(ARDUINO_ARCH_ESP32)
  bool loadCalibration(const char *namespaceName);
  bool saveCalibration(const char *namespaceName) const;
  bool clearStoredCalibration(const char *namespaceName) const;
#endif

  bool rememberCurrentAsFirstLimit();
  bool rememberCurrentAsSecondLimit();
  void rememberFirstLimit(float x, float y, float z);
  void rememberSecondLimit(float x, float y, float z);

  bool hasCalibration() const;
  bool isActive() const;
  bool justActivated() const;
  bool justDeactivated() const;
  float positionPercent() const;
  float filteredValue() const;
  const MagnetAxisTrackerState &state() const;

private:
  float selectValue(float x, float y, float z) const;
  void applySampleToCalibration(float value);
  void rebuildCalibrationFromLimits();
  void updateOutputState();

  MagnetAxisTrackerConfig _config;
  MagnetAxisTrackerState _state;

  bool _hasSample = false;
  bool _hasAnyCalibrationValue = false;
  bool _hasFirstLimit = false;
  bool _hasSecondLimit = false;
  bool _active = false;

  float _filtered = 0.0f;
  float _minValue = 0.0f;
  float _maxValue = 0.0f;
  float _firstLimit = 0.0f;
  float _secondLimit = 0.0f;
};

#endif
