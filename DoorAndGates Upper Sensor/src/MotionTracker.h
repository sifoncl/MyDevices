#ifndef MOTION_TRACKER_H
#define MOTION_TRACKER_H

#include <Arduino.h>
#include <math.h>

#include "SensorConfig.h"

class MotionTracker
{
public:
  void update(float value, unsigned long nowMs)
  {
    if (!_initialized)
    {
      _filtered = value;
      _previousFiltered = value;
      _initialized = true;
      pushHistory(value);
      return;
    }

    _filtered += (value - _filtered) * DIRECTION_FILTER_ALPHA;
    const float instantVelocity = _filtered - _previousFiltered;
    _previousFiltered = _filtered;
    _velocity += (instantVelocity - _velocity) * DIRECTION_VELOCITY_ALPHA;

    if (_historyCount < DIRECTION_WINDOW_SAMPLES)
    {
      pushHistory(_filtered);
      return;
    }

    const float previousWindowValue = _history[_historyIndex];
    pushHistory(_filtered);
    const float windowChange = _filtered - previousWindowValue;
    const int8_t candidateDirection =
        fabsf(windowChange) >= DIRECTION_START_THRESHOLD_UT
            ? (windowChange > 0.0f ? 1 : -1)
            : 0;

    if (!_moving)
    {
      updateCandidate(candidateDirection);
      if (_candidateCount >= DIRECTION_CONFIRM_SAMPLES)
      {
        _moving = true;
        _direction = _candidateDirection;
        _lastMovementAt = nowMs;
        _candidateCount = 0;
      }
      return;
    }

    const bool movementVisible =
        fabsf(windowChange) >= DIRECTION_CONTINUE_THRESHOLD_UT ||
        fabsf(_velocity) >= DIRECTION_VELOCITY_THRESHOLD_UT;

    if (movementVisible)
    {
      _lastMovementAt = nowMs;
    }
    else if ((nowMs - _lastMovementAt) >= DIRECTION_SETTLE_TIME_MS)
    {
      _moving = false;
      _direction = 0;
      _candidateDirection = 0;
      _candidateCount = 0;
    }
  }

  float filtered() const
  {
    return _filtered;
  }

  float velocity() const
  {
    return _velocity;
  }

  int8_t direction() const
  {
    return _direction;
  }

  bool moving() const
  {
    return _moving;
  }

private:
  void pushHistory(float value)
  {
    _history[_historyIndex] = value;
    _historyIndex = (_historyIndex + 1) % DIRECTION_WINDOW_SAMPLES;
    if (_historyCount < DIRECTION_WINDOW_SAMPLES)
    {
      _historyCount++;
    }
  }

  void updateCandidate(int8_t direction)
  {
    if (direction == 0)
    {
      _candidateDirection = 0;
      _candidateCount = 0;
      return;
    }

    if (direction != _candidateDirection)
    {
      _candidateDirection = direction;
      _candidateCount = 1;
      return;
    }

    if (_candidateCount < 255)
    {
      _candidateCount++;
    }
  }

  float _history[DIRECTION_WINDOW_SAMPLES] = {};
  uint8_t _historyIndex = 0;
  uint8_t _historyCount = 0;
  float _filtered = 0.0f;
  float _previousFiltered = 0.0f;
  float _velocity = 0.0f;
  int8_t _candidateDirection = 0;
  int8_t _direction = 0;
  uint8_t _candidateCount = 0;
  bool _initialized = false;
  bool _moving = false;
  unsigned long _lastMovementAt = 0;
};

#endif
