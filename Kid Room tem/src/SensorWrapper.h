#ifndef SENSOR_WRAPPER_H
#define SENSOR_WRAPPER_H

#include <Arduino.h>
#include <math.h>

template <typename TSensor>
class SensorWrapper
{
public:
  using ReadCallback = float (*)(TSensor &sensor);

  SensorWrapper(TSensor &sensor, ReadCallback readCallback, unsigned long updateIntervalMs, unsigned long unavailableDelayMs)
      : _sensor(sensor),
        _readCallback(readCallback),
        _updateIntervalMs(updateIntervalMs),
        _unavailableDelayMs(unavailableDelayMs),
        _lastUpdateAt(0),
        _unavailableStartedAt(0),
        _value(0),
        _lastRawValue(0),
        _minValue(0),
        _maxValue(0),
        _hasMinValue(false),
        _hasMaxValue(false),
        _hasValue(false),
        _available(false),
        _unavailableTimerStarted(false)
  {
  }

  bool loop()
  {
    if ((millis() - _lastUpdateAt) <= _updateIntervalMs)
    {
      return false;
    }

    return update();
  }

  bool update()
  {
    _lastUpdateAt = millis();

    if (_readCallback == nullptr)
    {
      markInvalid();
      return true;
    }

    const float newValue = _readCallback(_sensor);
    _lastRawValue = newValue;

    if (isValid(newValue))
    {
      _value = newValue;
      _hasValue = true;
      _available = true;
      _unavailableTimerStarted = false;
      _unavailableStartedAt = 0;
      return true;
    }

    markInvalid();
    return true;
  }

  float value() const
  {
    return _value;
  }

  float lastRawValue() const
  {
    return _lastRawValue;
  }

  bool hasValue() const
  {
    return _hasValue;
  }

  bool isAvailable() const
  {
    return _available;
  }

  void setUpdateInterval(unsigned long updateIntervalMs)
  {
    _updateIntervalMs = updateIntervalMs;
  }

  void setUnavailableDelay(unsigned long unavailableDelayMs)
  {
    _unavailableDelayMs = unavailableDelayMs;
  }

  void setMinValue(float minValue)
  {
    _minValue = minValue;
    _hasMinValue = true;
  }

  void setMaxValue(float maxValue)
  {
    _maxValue = maxValue;
    _hasMaxValue = true;
  }

  void setValidRange(float minValue, float maxValue)
  {
    setMinValue(minValue);
    setMaxValue(maxValue);
  }

  void clearValidRange()
  {
    _hasMinValue = false;
    _hasMaxValue = false;
  }

private:
  TSensor &_sensor;
  ReadCallback _readCallback;
  unsigned long _updateIntervalMs;
  unsigned long _unavailableDelayMs;
  unsigned long _lastUpdateAt;
  unsigned long _unavailableStartedAt;
  float _value;
  float _lastRawValue;
  float _minValue;
  float _maxValue;
  bool _hasMinValue;
  bool _hasMaxValue;
  bool _hasValue;
  bool _available;
  bool _unavailableTimerStarted;

  bool isValid(float newValue) const
  {
    if (isnan(newValue) || isinf(newValue))
    {
      return false;
    }

    if (_hasMinValue && newValue < _minValue)
    {
      return false;
    }

    if (_hasMaxValue && newValue > _maxValue)
    {
      return false;
    }

    return true;
  }

  void markInvalid()
  {
    const unsigned long nowMs = millis();

    if (!_unavailableTimerStarted)
    {
      _unavailableTimerStarted = true;
      _unavailableStartedAt = nowMs;
    }

    if ((nowMs - _unavailableStartedAt) > _unavailableDelayMs)
    {
      _available = false;
    }
  }
};

#endif
