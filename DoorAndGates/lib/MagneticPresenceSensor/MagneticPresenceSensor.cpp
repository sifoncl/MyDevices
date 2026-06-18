#include "MagneticPresenceSensor.h"

#include <math.h>

MagneticPresenceSensor::MagneticPresenceSensor(const MagneticPresenceSensorConfig &config)
    : _config(config)
{
  _state.sensitivity = _config.sensitivity;
  _state.axis = _config.axis;
  _state.invertDirection = _config.invertDirection;
}

MagneticPresenceSensorState MagneticPresenceSensor::update(float x, float y, float z)
{
  updateMeasurementState(x, y, z);
  updateMotionState(millis());
  return _state;
}

MagneticPresenceSensorState MagneticPresenceSensor::updateRemote(float x,
                                                                 float y,
                                                                 float z,
                                                                 int8_t direction,
                                                                 bool moving,
                                                                 float velocity)
{
  updateMeasurementState(x, y, z);
  updateRemoteMotionState(direction, moving, velocity);
  return _state;
}

void MagneticPresenceSensor::updateMeasurementState(float x, float y, float z)
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

  _state.x = x;
  _state.y = y;
  _state.z = z;
  _state.raw = raw;
  _state.filtered = _filtered;
  _state.valid = true;
  _state.sensitivity = _config.sensitivity;
  _state.axis = _config.axis;
  _state.invertDirection = _config.invertDirection;

  updateBaseline(_filtered);
  updatePresenceState();
}

void MagneticPresenceSensor::markInvalid()
{
  _state.valid = false;
  _state.justActivated = false;
  _state.justDeactivated = _active;
  _state.justMotionStarted = false;
  _state.justSettled = false;
  _state.active = false;
  _state.moving = false;
  _state.motionQualified = false;
  _state.direction = 0;
  _state.settledDirection = 0;
  _active = false;
  _moving = false;
  _motionQualified = false;
}

void MagneticPresenceSensor::resetCalibration()
{
  _state.calibrated = false;
  _state.baseline = 0.0f;
  _state.delta = 0.0f;
  _state.active = false;
  _state.justActivated = false;
  _state.justDeactivated = false;
  _state.justMotionStarted = false;
  _state.justSettled = false;
  _state.moving = false;
  _state.motionQualified = false;
  _state.direction = 0;
  _state.settledDirection = 0;
  _baselineSum = 0.0f;
  _baselineSampleCount = 0;
  _active = false;
  _moving = false;
  _motionQualified = false;
}

bool MagneticPresenceSensor::rememberCurrentAsBaseline()
{
  if (!_hasSample)
  {
    return false;
  }

  _state.baseline = _filtered;
  _state.delta = 0.0f;
  _state.calibrated = true;
  _baselineSum = 0.0f;
  _baselineSampleCount = _config.baselineSamples;
  updatePresenceState();
  return true;
}

void MagneticPresenceSensor::setBaseline(float baseline)
{
  _state.baseline = baseline;
  _state.delta = _hasSample ? fabsf(_filtered - _state.baseline) : 0.0f;
  _state.calibrated = true;
  _baselineSum = 0.0f;
  _baselineSampleCount = _config.baselineSamples;
  updatePresenceState();
}

void MagneticPresenceSensor::setSensitivity(float sensitivity)
{
  _config.sensitivity = max(1.0f, sensitivity);
  _state.sensitivity = _config.sensitivity;
  updatePresenceState();
}

void MagneticPresenceSensor::setAxis(MagneticPresenceAxis axis)
{
  if (_config.axis == axis)
  {
    return;
  }

  _config.axis = axis;
  _state.axis = axis;
  _hasSample = false;
  _hasPreviousFiltered = false;
  resetCalibration();
}

void MagneticPresenceSensor::setInvertDirection(bool inverted)
{
  _config.invertDirection = inverted;
  _state.invertDirection = inverted;
}

void MagneticPresenceSensor::setMotionThreshold(float threshold)
{
  _config.motionThreshold = max(0.1f, threshold);
}

void MagneticPresenceSensor::setDirectionMinChange(float minChange)
{
  _config.directionMinChange = max(0.1f, minChange);
}

void MagneticPresenceSensor::setSettleTime(unsigned long settleTimeMs)
{
  _config.settleTimeMs = max(100UL, settleTimeMs);
}

float MagneticPresenceSensor::sensitivity() const
{
  return _config.sensitivity;
}

float MagneticPresenceSensor::baseline() const
{
  return _state.baseline;
}

MagneticPresenceAxis MagneticPresenceSensor::axis() const
{
  return _config.axis;
}

bool MagneticPresenceSensor::invertDirection() const
{
  return _config.invertDirection;
}

bool MagneticPresenceSensor::isActive() const
{
  return _state.active;
}

bool MagneticPresenceSensor::isCalibrated() const
{
  return _state.calibrated;
}

const MagneticPresenceSensorState &MagneticPresenceSensor::state() const
{
  return _state;
}

float MagneticPresenceSensor::selectValue(float x, float y, float z) const
{
  switch (_config.axis)
  {
  case MagneticPresenceAxis::X:
    return x;
  case MagneticPresenceAxis::Y:
    return y;
  case MagneticPresenceAxis::Z:
    return z;
  }

  return 0.0f;
}

int8_t MagneticPresenceSensor::directionFromDelta(float delta) const
{
  if (fabsf(delta) < _config.directionMinChange)
  {
    return 0;
  }

  int8_t direction = delta > 0.0f ? 1 : -1;
  if (_config.invertDirection)
  {
    direction = -direction;
  }
  return direction;
}

void MagneticPresenceSensor::updateBaseline(float filtered)
{
  if (!_state.calibrated)
  {
    _baselineSum += filtered;
    _baselineSampleCount++;

    if (_baselineSampleCount >= max<uint8_t>(1, _config.baselineSamples))
    {
      _state.baseline = _baselineSum / _baselineSampleCount;
      _state.calibrated = true;
    }
    return;
  }

  if (_active || _moving)
  {
    return;
  }

  const float trackingSamples = max<uint16_t>(1, _config.baselineTrackingSamples);
  _state.baseline += (filtered - _state.baseline) / trackingSamples;
}

void MagneticPresenceSensor::updatePresenceState()
{
  _state.justActivated = false;
  _state.justDeactivated = false;

  if (!_state.valid || !_state.calibrated)
  {
    _state.delta = 0.0f;
    _state.active = false;
    _active = false;
    return;
  }

  _state.delta = fabsf(_state.filtered - _state.baseline);

  const float sensitivity = max(1.0f, _config.sensitivity);
  const float releaseThreshold = max(0.5f, sensitivity - max(0.0f, _config.hysteresis));

  bool nextActive = _active;
  if (_active)
  {
    nextActive = _state.delta >= releaseThreshold;
  }
  else
  {
    nextActive = _state.delta >= sensitivity;
  }

  _state.active = nextActive;
  _state.justActivated = !_active && nextActive;
  _state.justDeactivated = _active && !nextActive;
  _active = nextActive;
}

void MagneticPresenceSensor::updateMotionState(unsigned long nowMs)
{
  _state.justMotionStarted = false;
  _state.justSettled = false;
  _state.settledDirection = 0;

  if (!_hasPreviousFiltered)
  {
    _previousFiltered = _filtered;
    _hasPreviousFiltered = true;
    _state.velocity = 0.0f;
    return;
  }

  const float velocity = _filtered - _previousFiltered;
  _previousFiltered = _filtered;
  _state.velocity = velocity;

  const bool meaningfulMovement = fabsf(velocity) >= _config.motionThreshold;

  if (!_moving && meaningfulMovement)
  {
    _moving = true;
    _motionQualified = _state.active;
    _motionStartValue = _filtered - velocity;
    _lastMovementAt = nowMs;
    _state.justMotionStarted = true;
  }

  if (_moving)
  {
    _motionQualified = _motionQualified || _state.active;

    if (meaningfulMovement)
    {
      _lastMovementAt = nowMs;
    }

    const float totalChange = _filtered - _motionStartValue;
    const int8_t direction = directionFromDelta(totalChange);
    if (direction != 0)
    {
      _state.direction = direction;
    }

    if ((nowMs - _lastMovementAt) >= _config.settleTimeMs)
    {
      _moving = false;
      const int8_t settledDirection = direction != 0 ? direction : _state.direction;
      _state.justSettled = _motionQualified && settledDirection != 0;
      _state.settledDirection = _state.justSettled ? settledDirection : 0;
    }
  }

  _state.moving = _moving;
  _state.motionQualified = _motionQualified;
  if (!_moving && !_state.justSettled)
  {
    _state.direction = 0;
    _motionQualified = false;
    _state.motionQualified = false;
  }
}

void MagneticPresenceSensor::updateRemoteMotionState(int8_t direction,
                                                     bool moving,
                                                     float velocity)
{
  const bool wasMoving = _moving;
  const int8_t previousDirection = _state.direction;

  direction = constrain(direction, -1, 1);
  if (_config.invertDirection)
  {
    direction = -direction;
  }

  _state.justMotionStarted = false;
  _state.justSettled = false;
  _state.settledDirection = 0;
  _state.velocity = velocity;

  if (moving)
  {
    _moving = true;
    _motionQualified = wasMoving ? _motionQualified : _state.active;
    _motionQualified = _motionQualified || _state.active;
    _state.justMotionStarted = !wasMoving;
    if (direction != 0)
    {
      _state.direction = direction;
    }
  }
  else
  {
    _moving = false;
    if (wasMoving && _motionQualified && previousDirection != 0)
    {
      _state.justSettled = true;
      _state.settledDirection = previousDirection;
    }
    _state.direction = 0;
    _motionQualified = false;
  }

  _state.moving = _moving;
  _state.motionQualified = _motionQualified;
}
