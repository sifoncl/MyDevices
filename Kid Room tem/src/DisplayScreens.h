#ifndef DISPLAY_SCREENS_H
#define DISPLAY_SCREENS_H

#include <Arduino.h>
#include <Adafruit_BME280.h>
#include <Adafruit_SSD1306.h>
#include <SensirionI2cScd4x.h>
#include <WiFi.h>

#include "TouchButton.h"
#include "InternetClock.h"
#include "SensorWrapper.h"

class DisplayScreen
{
public:
  virtual ~DisplayScreen() = default;
  virtual void draw(Adafruit_SSD1306 &display) = 0;
};

class StartupIpScreen : public DisplayScreen
{
public:
  void setIp(IPAddress ip);
  void setDeviceName(const char *deviceName);
  void setDeviceInfo(const char *deviceId, const char *softwareVersion);
  void draw(Adafruit_SSD1306 &display) override;

private:
  IPAddress _ip;
  String _deviceName;
  String _deviceId;
  String _softwareVersion;
};

class BigClockScreen : public DisplayScreen
{
public:
  explicit BigClockScreen(InternetClock &clock);
  void draw(Adafruit_SSD1306 &display) override;

private:
  InternetClock &_clock;
};

class DateClockScreen : public DisplayScreen
{
public:
  explicit DateClockScreen(InternetClock &clock);
  void draw(Adafruit_SSD1306 &display) override;

private:
  InternetClock &_clock;
};

template <typename TSensor>
class SensorValueScreen : public DisplayScreen
{
public:
  SensorValueScreen(SensorWrapper<TSensor> &sensor, const char *title, const char *unit, uint8_t decimals, uint8_t unitTextSize = 2)
      : _sensor(sensor), _title(title), _unit(unit), _decimals(decimals), _unitTextSize(unitTextSize)
  {
  }

  void draw(Adafruit_SSD1306 &display) override
  {
    display.setTextColor(WHITE);
    drawCenteredText(display, _title, 2, 0);
    display.drawLine(0, 18, display.width() - 1, 18, WHITE);

    if (!_sensor.isAvailable() || !_sensor.hasValue())
    {
      drawCenteredText(display, "--", 4, 28);
      return;
    }

    const String valueText = String(_sensor.value(), static_cast<unsigned int>(_decimals));
    drawValueWithUnit(display, valueText, _unit, 30);
  }

private:
  SensorWrapper<TSensor> &_sensor;
  const char *_title;
  const char *_unit;
  uint8_t _decimals;
  uint8_t _unitTextSize;

  static void drawCenteredText(Adafruit_SSD1306 &display, const String &text, uint8_t textSize, int16_t y)
  {
    int16_t x1 = 0;
    int16_t y1 = 0;
    uint16_t width = 0;
    uint16_t height = 0;

    display.setTextSize(textSize);
    display.getTextBounds(text, 0, y, &x1, &y1, &width, &height);
    display.setCursor((display.width() - static_cast<int16_t>(width)) / 2 - x1, y);
    display.println(text);
  }

  static uint16_t textWidth(Adafruit_SSD1306 &display, const String &text, uint8_t textSize)
  {
    int16_t x1 = 0;
    int16_t y1 = 0;
    uint16_t width = 0;
    uint16_t height = 0;

    display.setTextSize(textSize);
    display.getTextBounds(text, 0, 0, &x1, &y1, &width, &height);
    return width;
  }

  void drawValueWithUnit(Adafruit_SSD1306 &display, const String &valueText, const char *unit, int16_t y)
  {
    const uint8_t valueTextSize = 3;
    const uint8_t unitTextSize = _unitTextSize;
    const int16_t gap = 4;
    const uint16_t valueWidth = textWidth(display, valueText, valueTextSize);
    const uint16_t unitWidth = textWidth(display, unit, unitTextSize);
    const int16_t totalWidth = valueWidth + gap + unitWidth;
    const int16_t x = (display.width() - totalWidth) / 2;

    display.setTextSize(valueTextSize);
    display.setCursor(x, y);
    display.print(valueText);

    display.setTextSize(unitTextSize);
    display.setCursor(x + valueWidth + gap, y + (unitTextSize == 1 ? 15 : 8));
    display.print(unit);
  }
};

class ScreenController
{
public:
  using EnabledChangedCallback = void (*)(bool enabled);

  ScreenController(Adafruit_SSD1306 &display, TouchButton &button, StartupIpScreen &ipScreen, DisplayScreen **screens, uint8_t screenCount);

  void begin(IPAddress ip, bool showStartupIp = true);
  void loop();
  void nextScreen();
  void showIpScreen();
  void setEnabled(bool enabled);
  void setOnEnabledChanged(EnabledChangedCallback callback);
  void setAlarmMode(bool active, uint8_t screenIndex);
  bool isEnabled() const;
  bool isAlarmMode() const;

private:
  static const unsigned long STARTUP_IP_SCREEN_MS = 5000UL;
  static const unsigned long LONG_PRESS_MS = 10000UL;
  static const unsigned long RENDER_INTERVAL_MS = 500UL;
  static const unsigned long ALARM_INVERT_INTERVAL_MS = 1000UL;

  Adafruit_SSD1306 &_display;
  TouchButton &_button;
  StartupIpScreen &_ipScreen;
  DisplayScreen **_screens;
  uint8_t _screenCount;
  uint8_t _currentScreen;
  bool _buttonWasPressed;
  bool _longPressHandled;
  bool _enabled;
  bool _showingIpScreen;
  bool _startupIpActive;
  bool _alarmMode;
  bool _alarmInvertState;
  bool _enabledBeforeAlarm;
  bool _showingIpBeforeAlarm;
  bool _startupIpActiveBeforeAlarm;
  unsigned long _startupIpUntil;
  unsigned long _startupIpUntilBeforeAlarm;
  unsigned long _lastRenderAt;
  unsigned long _lastAlarmInvertAt;
  uint8_t _alarmScreen;
  uint8_t _screenBeforeAlarm;
  EnabledChangedCallback _onEnabledChanged;

  DisplayScreen *activeScreen();
  void render(bool force = false);
  void showMainScreen(uint8_t index);
};

#endif
