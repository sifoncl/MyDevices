#ifndef TOUCH_BUTTON_H
#define TOUCH_BUTTON_H

#include <Arduino.h>

class TouchButton
{
public:
  TouchButton(uint8_t pin,
              uint16_t thresholdDelta = 20,
              unsigned long debounceMs = 50,
              void (*onPress)() = nullptr,
              void (*onRelease)() = nullptr)
      : _pin(pin),
        _thresholdDelta(thresholdDelta),
        _debounceTime(debounceMs),
        _baseline(0),
        _pressThreshold(0),
        _releaseThreshold(0),
        _lastTouchValue(0),
        _filteredTouchValue(0),
        _filteredInitialized(false),
        _lastRawState(false),
        _state(false),
        _lastChangeTime(0),
        _pressStartTime(0),
        _onPressCallback(onPress),
        _onReleaseCallback(onRelease)
  {
  }

  void begin(uint8_t calibrationSamples = 30)
  {
    calibrate(calibrationSamples);

    const uint16_t initialValue = readMedianTouchValue();
    _lastTouchValue = initialValue;
    if (isValidTouchValue(initialValue))
    {
      _filteredTouchValue = initialValue;
      _filteredInitialized = true;
    }

    const bool raw = readRawState();
    _lastRawState = raw;
    _state = raw;
    _lastChangeTime = millis();
    _pressStartTime = _state ? _lastChangeTime : 0;
  }

  void calibrate(uint8_t samples = 30)
  {
    if (samples == 0)
    {
      samples = 1;
    }

    touchRead(_pin);
    delay(20);

    uint16_t values[60];
    uint8_t validSamples = 0;
    const uint8_t samplesToRead = samples > 60 ? 60 : samples;

    for (uint8_t i = 0; i < samplesToRead; i++)
    {
      const uint16_t value = touchRead(_pin);
      if (isValidTouchValue(value))
      {
        values[validSamples++] = value;
      }
      delay(5);
    }

    if (validSamples == 0)
    {
      _baseline = 0;
      _pressThreshold = 0;
      _releaseThreshold = 0;
      _filteredTouchValue = 0;
      _filteredInitialized = false;
      return;
    }

    sortValues(values, validSamples);
    _baseline = values[validSamples / 2];
    _filteredTouchValue = _baseline;
    _filteredInitialized = true;
    updateThresholds();
  }

  void forceReleased()
  {
    _lastRawState = false;
    _state = false;
    _lastChangeTime = millis();
    _pressStartTime = 0;
  }

  void recalibrate()
  {
    calibrate();
    forceReleased();
  }

  void setThresholdDelta(uint16_t thresholdDelta)
  {
    _thresholdDelta = thresholdDelta;
    updateThresholds();
  }

  void setOnPress(void (*callback)())
  {
    _onPressCallback = callback;
  }

  void setOnRelease(void (*callback)())
  {
    _onReleaseCallback = callback;
  }

  void update()
  {
    const bool raw = readRawState();

    if (raw != _lastRawState)
    {
      _lastChangeTime = millis();
      _lastRawState = raw;
    }

    if ((millis() - _lastChangeTime) >= _debounceTime && raw != _state)
    {
      _state = raw;

      if (_state)
      {
        _pressStartTime = millis();
        if (_onPressCallback != nullptr)
        {
          _onPressCallback();
        }
      }
      else if (_onReleaseCallback != nullptr)
      {
        _onReleaseCallback();
      }
    }
  }

  bool isPressed() const
  {
    return _state;
  }

  bool isReleased() const
  {
    return !_state;
  }

  bool pressedFor(unsigned long ms) const
  {
    if (!_state)
    {
      return false;
    }
    return (millis() - _pressStartTime) >= ms;
  }

  unsigned long pressDuration() const
  {
    return _state ? (millis() - _pressStartTime) : 0;
  }

  uint16_t lastTouchValue() const
  {
    return _lastTouchValue;
  }

  uint16_t baseline() const
  {
    return _baseline;
  }

  uint16_t threshold() const
  {
    return _pressThreshold;
  }

  uint16_t releaseThreshold() const
  {
    return _releaseThreshold;
  }

  uint16_t filteredValue() const
  {
    return _filteredTouchValue;
  }

private:
  uint8_t _pin;
  uint16_t _thresholdDelta;
  unsigned long _debounceTime;
  uint16_t _baseline;
  uint16_t _pressThreshold;
  uint16_t _releaseThreshold;
  uint16_t _lastTouchValue;
  uint16_t _filteredTouchValue;
  bool _filteredInitialized;
  bool _lastRawState;
  bool _state;
  unsigned long _lastChangeTime;
  unsigned long _pressStartTime;
  void (*_onPressCallback)();
  void (*_onReleaseCallback)();

  static const uint16_t MAX_VALID_TOUCH_VALUE = 65000;
  static const uint8_t READ_SAMPLES = 7;
  static const uint8_t FILTER_DIVIDER = 4;
  static const uint16_t MIN_BASELINE_TRACK_WINDOW = 1200;

  bool readRawState()
  {
    _lastTouchValue = readMedianTouchValue();

    if (!isValidTouchValue(_lastTouchValue))
    {
      return false;
    }

    if (isLowOutlier(_lastTouchValue))
    {
      return _state ? (_filteredTouchValue >= _releaseThreshold) : false;
    }

    updateFilteredValue(_lastTouchValue);

    if (_baseline == 0 || !isValidTouchValue(_baseline))
    {
      _baseline = _filteredTouchValue;
      if (_baseline > 0)
      {
        updateThresholds();
      }
      return false;
    }

    if (!_state && isWithinBaselineTrackWindow(_filteredTouchValue))
    {
      updateBaseline(_filteredTouchValue);
    }

    const uint16_t activeThreshold = _state ? _releaseThreshold : _pressThreshold;
    return _filteredTouchValue >= activeThreshold;
  }

  void updateThresholds()
  {
    const uint32_t pressThreshold = static_cast<uint32_t>(_baseline) + _thresholdDelta;
    _pressThreshold = pressThreshold > UINT16_MAX ? UINT16_MAX : static_cast<uint16_t>(pressThreshold);

    uint16_t releaseDelta = _thresholdDelta / 2;
    if (_thresholdDelta > 0 && releaseDelta == 0)
    {
      releaseDelta = 1;
    }

    const uint32_t releaseThreshold = static_cast<uint32_t>(_baseline) + releaseDelta;
    _releaseThreshold = releaseThreshold > UINT16_MAX ? UINT16_MAX : static_cast<uint16_t>(releaseThreshold);
  }

  void updateBaseline(uint16_t value)
  {
    if (!isValidTouchValue(value))
    {
      return;
    }

    _baseline = static_cast<uint16_t>((static_cast<uint32_t>(_baseline) * 31UL + value) / 32UL);
    updateThresholds();
  }

  void updateFilteredValue(uint16_t value)
  {
    if (!_filteredInitialized)
    {
      _filteredTouchValue = value;
      _filteredInitialized = true;
      return;
    }

    _filteredTouchValue = static_cast<uint16_t>(
        (static_cast<uint32_t>(_filteredTouchValue) * (FILTER_DIVIDER - 1) + value) / FILTER_DIVIDER);
  }

  static bool isValidTouchValue(uint16_t value)
  {
    return value > 0 && value < MAX_VALID_TOUCH_VALUE;
  }

  uint16_t baselineTrackWindow() const
  {
    const uint16_t dynamicWindow = _thresholdDelta / 4;
    return dynamicWindow > MIN_BASELINE_TRACK_WINDOW ? dynamicWindow : MIN_BASELINE_TRACK_WINDOW;
  }

  bool isLowOutlier(uint16_t value) const
  {
    if (_baseline == 0 || !isValidTouchValue(_baseline))
    {
      return false;
    }

    const uint16_t window = baselineTrackWindow();
    return (static_cast<uint32_t>(value) + window) < _baseline;
  }

  bool isWithinBaselineTrackWindow(uint16_t value) const
  {
    if (_baseline == 0 || !isValidTouchValue(_baseline))
    {
      return true;
    }

    const uint16_t window = baselineTrackWindow();
    const uint16_t lowerBound = _baseline > window ? _baseline - window : 0;
    const uint32_t upperBound = static_cast<uint32_t>(_baseline) + window;

    return value >= lowerBound && value <= upperBound;
  }

  uint16_t readMedianTouchValue()
  {
    uint16_t values[READ_SAMPLES];
    uint8_t validSamples = 0;

    for (uint8_t i = 0; i < READ_SAMPLES; i++)
    {
      const uint16_t value = touchRead(_pin);
      if (isValidTouchValue(value))
      {
        values[validSamples++] = value;
      }
      delayMicroseconds(500);
    }

    if (validSamples == 0)
    {
      return UINT16_MAX;
    }

    sortValues(values, validSamples);
    return values[validSamples / 2];
  }

  static void sortValues(uint16_t *values, uint8_t count)
  {
    for (uint8_t i = 1; i < count; i++)
    {
      const uint16_t current = values[i];
      int8_t j = i - 1;

      while (j >= 0 && values[j] > current)
      {
        values[j + 1] = values[j];
        j--;
      }
      values[j + 1] = current;
    }
  }
};

#endif
