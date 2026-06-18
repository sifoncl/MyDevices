#ifndef MAGNETIC_PRESENCE_SENSOR_H
#define MAGNETIC_PRESENCE_SENSOR_H

#include <Arduino.h>

enum class MagneticPresenceAxis : uint8_t
{
  X,
  Y,
  Z
};

struct MagneticPresenceSensorConfig
{
  MagneticPresenceAxis axis = MagneticPresenceAxis::X;
  bool invertDirection = false;
  float sensitivity = 80.0f;
  float hysteresis = 15.0f;
  float motionThreshold = 1.5f;
  float directionMinChange = 12.0f;
  unsigned long settleTimeMs = 1200UL;
  uint8_t smoothingSamples = 4;
  uint8_t baselineSamples = 25;
  uint16_t baselineTrackingSamples = 80;
};

struct MagneticPresenceSensorState
{
  float x = 0.0f;
  float y = 0.0f;
  float z = 0.0f;
  float raw = 0.0f;
  float filtered = 0.0f;
  float baseline = 0.0f;
  float delta = 0.0f;
  float velocity = 0.0f;
  float sensitivity = 0.0f;
  MagneticPresenceAxis axis = MagneticPresenceAxis::X;
  bool invertDirection = false;
  bool valid = false;
  bool calibrated = false;
  bool active = false;
  bool justActivated = false;
  bool justDeactivated = false;
  bool moving = false;
  bool justMotionStarted = false;
  bool justSettled = false;
  bool motionQualified = false;
  int8_t direction = 0;
  int8_t settledDirection = 0;
};

class MagneticPresenceSensor
{
public:
  explicit MagneticPresenceSensor(const MagneticPresenceSensorConfig &config = MagneticPresenceSensorConfig());

  MagneticPresenceSensorState update(float x, float y, float z);
  MagneticPresenceSensorState updateRemote(float x,
                                           float y,
                                           float z,
                                           int8_t direction,
                                           bool moving,
                                           float velocity);
  void markInvalid();
  void resetCalibration();
  bool rememberCurrentAsBaseline();
  void setBaseline(float baseline);

  void setSensitivity(float sensitivity);
  void setAxis(MagneticPresenceAxis axis);
  void setInvertDirection(bool inverted);
  void setMotionThreshold(float threshold);
  void setDirectionMinChange(float minChange);
  void setSettleTime(unsigned long settleTimeMs);

  float sensitivity() const;
  float baseline() const;
  MagneticPresenceAxis axis() const;
  bool invertDirection() const;
  bool isActive() const;
  bool isCalibrated() const;
  const MagneticPresenceSensorState &state() const;

private:
  float selectValue(float x, float y, float z) const;
  int8_t directionFromDelta(float delta) const;
  void updateMeasurementState(float x, float y, float z);
  void updateBaseline(float filtered);
  void updatePresenceState();
  void updateMotionState(unsigned long nowMs);
  void updateRemoteMotionState(int8_t direction, bool moving, float velocity);

  MagneticPresenceSensorConfig _config;
  MagneticPresenceSensorState _state;

  bool _hasSample = false;
  bool _hasPreviousFiltered = false;
  bool _active = false;
  bool _moving = false;
  bool _motionQualified = false;
  float _filtered = 0.0f;
  float _previousFiltered = 0.0f;
  float _motionStartValue = 0.0f;
  float _baselineSum = 0.0f;
  uint8_t _baselineSampleCount = 0;
  unsigned long _lastMovementAt = 0;
};

#endif
