#include "DisplayScreens.h"
#include <U8g2_for_Adafruit_GFX.h>

namespace
{
U8G2_FOR_ADAFRUIT_GFX startupTextRenderer;
bool startupTextRendererInitialized = false;

bool wifiConnected()
{
  return WiFi.status() == WL_CONNECTED;
}

void drawHeader(Adafruit_SSD1306 &display, const char *title)
{
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(title);
  display.drawLine(0, 10, display.width() - 1, 10, WHITE);
}

void drawCenteredText(Adafruit_SSD1306 &display, const String &text, uint8_t textSize, int16_t y)
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

U8G2_FOR_ADAFRUIT_GFX &utf8FontRenderer(Adafruit_SSD1306 &display)
{
  if (!startupTextRendererInitialized)
  {
    startupTextRenderer.begin(display);
    startupTextRendererInitialized = true;
  }
  else
  {
    startupTextRenderer.begin(display);
  }

  startupTextRenderer.setFontMode(1);
  startupTextRenderer.setForegroundColor(WHITE);
  startupTextRenderer.setBackgroundColor(BLACK);
  startupTextRenderer.setFont(u8g2_font_5x8_t_cyrillic);
  return startupTextRenderer;
}

size_t utf8CharLength(const char *text, size_t offset)
{
  const uint8_t lead = static_cast<uint8_t>(text[offset]);

  if ((lead & 0x80U) == 0)
  {
    return 1;
  }

  if ((lead & 0xE0U) == 0xC0U)
  {
    return 2;
  }

  if ((lead & 0xF0U) == 0xE0U)
  {
    return 3;
  }

  if ((lead & 0xF8U) == 0xF0U)
  {
    return 4;
  }

  return 1;
}

int16_t utf8LineHeight(U8G2_FOR_ADAFRUIT_GFX &fontRenderer)
{
  return fontRenderer.getFontAscent() - fontRenderer.getFontDescent() + 1;
}

uint8_t drawWrappedUtf8(U8G2_FOR_ADAFRUIT_GFX &fontRenderer,
                       const String &text,
                       int16_t x,
                       int16_t firstBaselineY,
                       int16_t maxWidth,
                       uint8_t maxLines)
{
  if (text.length() == 0 || maxLines == 0)
  {
    return 0;
  }

  const char *rawText = text.c_str();
  const size_t textLength = strlen(rawText);
  size_t start = 0;
  uint8_t linesDrawn = 0;
  const int16_t lineHeight = utf8LineHeight(fontRenderer);

  while (start < textLength && linesDrawn < maxLines)
  {
    while (start < textLength && rawText[start] == ' ')
    {
      start++;
    }

    if (start >= textLength)
    {
      break;
    }

    size_t end = start;
    size_t bestBreak = start;

    while (end < textLength)
    {
      const size_t charLength = utf8CharLength(rawText, end);
      const size_t next = end + charLength <= textLength ? end + charLength : textLength;
      const String candidate = text.substring(start, next);

      if (fontRenderer.getUTF8Width(candidate.c_str()) > maxWidth)
      {
        if (bestBreak == start)
        {
          bestBreak = next;
        }
        break;
      }

      bestBreak = next;
      if (charLength == 1 && (rawText[end] == ' ' || rawText[end] == '-' || rawText[end] == '_'))
      {
        bestBreak = next;
      }

      end = next;
    }

    size_t lineEnd = bestBreak;
    while (lineEnd > start && rawText[lineEnd - 1] == ' ')
    {
      lineEnd--;
    }

    const String lineText = text.substring(start, lineEnd);
    fontRenderer.drawUTF8(x, firstBaselineY + static_cast<int16_t>(linesDrawn * lineHeight), lineText.c_str());

    linesDrawn++;
    start = bestBreak;
  }

  return linesDrawn;
}

void drawNoWifiMessage(Adafruit_SSD1306 &display)
{
  display.setTextColor(WHITE);
  drawCenteredText(display, "No WiFi", 2, 24);
}

void drawNoWifiTimeMessage(Adafruit_SSD1306 &display)
{
  display.setTextColor(WHITE);
  drawCenteredText(display, "No WiFi", 2, 18);
  drawCenteredText(display, "Time unavailable", 1, 42);
}
}

void StartupIpScreen::setIp(IPAddress ip)
{
  _ip = ip;
}

void StartupIpScreen::setDeviceName(const char *deviceName)
{
  _deviceName = deviceName == nullptr ? "" : deviceName;
}

void StartupIpScreen::setDeviceInfo(const char *deviceId, const char *softwareVersion)
{
  _deviceId = deviceId == nullptr ? "" : deviceId;
  _softwareVersion = softwareVersion == nullptr ? "" : softwareVersion;
}

void StartupIpScreen::draw(Adafruit_SSD1306 &display)
{
  drawHeader(display, wifiConnected() ? "WiFi connected" : "WiFi not connected");

  U8G2_FOR_ADAFRUIT_GFX &fontRenderer = utf8FontRenderer(display);
  drawWrappedUtf8(fontRenderer, _deviceName, 0, 19, display.width(), 3);

  const String idLine = String("ID: ") + _deviceId;
  const String versionLine = String("FW: ") + _softwareVersion;
  const String ipLine = wifiConnected() ? String("IP: ") + WiFi.localIP().toString() : String("IP: ");

  fontRenderer.drawUTF8(0, 43, idLine.c_str());
  fontRenderer.drawUTF8(0, 52, versionLine.c_str());
  fontRenderer.drawUTF8(0, 61, ipLine.c_str());
}

BigClockScreen::BigClockScreen(InternetClock &clock)
    : _clock(clock)
{
}

void BigClockScreen::draw(Adafruit_SSD1306 &display)
{
  if (!wifiConnected())
  {
    drawNoWifiTimeMessage(display);
    return;
  }

  if (!_clock.isSynced())
  {
    drawCenteredText(display, "Syncing time...", 1, 28);
    return;
  }

  drawCenteredText(display, _clock.shortTimeString(), 4, 18);
}

DateClockScreen::DateClockScreen(InternetClock &clock)
    : _clock(clock)
{
}

void DateClockScreen::draw(Adafruit_SSD1306 &display)
{
  if (!wifiConnected())
  {
    drawNoWifiTimeMessage(display);
    return;
  }

  if (!_clock.isSynced())
  {
    drawCenteredText(display, "Syncing time...", 1, 28);
    return;
  }

  drawCenteredText(display, _clock.timeString(), 2, 12);
  drawCenteredText(display, _clock.dateString(), 2, 40);
}

ScreenController::ScreenController(Adafruit_SSD1306 &display, TouchButton &button, StartupIpScreen &ipScreen, DisplayScreen **screens, uint8_t screenCount)
    : _display(display),
      _button(button),
      _ipScreen(ipScreen),
      _screens(screens),
      _screenCount(screenCount),
      _currentScreen(0),
      _buttonWasPressed(false),
      _longPressHandled(false),
      _enabled(false),
      _showingIpScreen(false),
      _startupIpActive(false),
      _alarmMode(false),
      _alarmInvertState(false),
      _enabledBeforeAlarm(false),
      _showingIpBeforeAlarm(false),
      _startupIpActiveBeforeAlarm(false),
      _startupIpUntil(0),
      _startupIpUntilBeforeAlarm(0),
      _lastRenderAt(0),
      _lastAlarmInvertAt(0),
      _alarmScreen(0),
      _screenBeforeAlarm(0),
      _onEnabledChanged(nullptr)
{
}

void ScreenController::begin(IPAddress ip, bool showStartupIp)
{
  _ipScreen.setIp(ip);

  if (showStartupIp)
  {
    _showingIpScreen = true;
    _startupIpActive = true;
    _startupIpUntil = millis() + STARTUP_IP_SCREEN_MS;
  }
  else
  {
    _showingIpScreen = false;
    _startupIpActive = false;
    _startupIpUntil = 0;
  }

  render(true);
}

void ScreenController::loop()
{
  _button.update();

  if (_alarmMode)
  {
    if (!_enabled)
    {
      setEnabled(true);
    }

    const unsigned long nowMs = millis();
    if ((nowMs - _lastAlarmInvertAt) >= ALARM_INVERT_INTERVAL_MS)
    {
      _lastAlarmInvertAt = nowMs;
      _alarmInvertState = !_alarmInvertState;
      _display.invertDisplay(_alarmInvertState);
    }

    render();
    return;
  }

  const bool buttonPressed = _button.isPressed();
  if (!_enabled)
  {
    if (!_buttonWasPressed && buttonPressed)
    {
      setEnabled(true);
      _buttonWasPressed = true;
      _longPressHandled = true;
    }
    else if (_buttonWasPressed && !buttonPressed)
    {
      _buttonWasPressed = false;
      _longPressHandled = false;
    }
    return;
  }

  if (buttonPressed && _button.pressedFor(LONG_PRESS_MS) && !_longPressHandled)
  {
    if (_showingIpScreen)
    {
      showMainScreen(_currentScreen);
    }
    else
    {
      showIpScreen();
    }
    _longPressHandled = true;
  }

  if (_buttonWasPressed && !buttonPressed)
  {
    if (!_longPressHandled)
    {
      nextScreen();
    }
    _longPressHandled = false;
  }
  _buttonWasPressed = buttonPressed;

  if (_startupIpActive && millis() >= _startupIpUntil)
  {
    _startupIpActive = false;
    showMainScreen(0);
  }

  render();
}

void ScreenController::nextScreen()
{
  if (!_enabled || _alarmMode)
  {
    return;
  }

  if (_screenCount == 0)
  {
    return;
  }

  showMainScreen((_currentScreen + 1) % _screenCount);
}

void ScreenController::showIpScreen()
{
  if (!_enabled || _alarmMode)
  {
    return;
  }

  _ipScreen.setIp(WiFi.localIP());
  _showingIpScreen = true;
  _startupIpActive = false;
  render(true);
}

void ScreenController::setEnabled(bool enabled)
{
  if (_alarmMode && !enabled)
  {
    _enabledBeforeAlarm = false;
    if (_onEnabledChanged != nullptr)
    {
      _onEnabledChanged(true);
    }
    return;
  }

  if (_enabled == enabled)
  {
    if (!enabled)
    {
      _display.clearDisplay();
      _display.display();
      _display.ssd1306_command(SSD1306_DISPLAYOFF);
    }
    return;
  }

  _enabled = enabled;
  _buttonWasPressed = false;
  _longPressHandled = false;

  if (_enabled)
  {
    _display.ssd1306_command(SSD1306_DISPLAYON);
    render(true);
    if (_onEnabledChanged != nullptr)
    {
      _onEnabledChanged(true);
    }
    return;
  }

  _display.clearDisplay();
  _display.display();
  _display.ssd1306_command(SSD1306_DISPLAYOFF);
  if (_onEnabledChanged != nullptr)
  {
    _onEnabledChanged(false);
  }
}

void ScreenController::setOnEnabledChanged(EnabledChangedCallback callback)
{
  _onEnabledChanged = callback;
}

bool ScreenController::isEnabled() const
{
  return _enabled;
}

void ScreenController::setAlarmMode(bool active, uint8_t screenIndex)
{
  if (_screenCount == 0)
  {
    return;
  }

  const uint8_t normalizedScreen = screenIndex % _screenCount;

  if (active)
  {
    if (_alarmMode)
    {
      _alarmScreen = normalizedScreen;
      _currentScreen = _alarmScreen;
      if (!_enabled)
      {
        setEnabled(true);
      }
      return;
    }

    _enabledBeforeAlarm = _enabled;
    _screenBeforeAlarm = _currentScreen;
    _showingIpBeforeAlarm = _showingIpScreen;
    _startupIpActiveBeforeAlarm = _startupIpActive;
    _startupIpUntilBeforeAlarm = _startupIpUntil;

    _alarmMode = true;
    _alarmScreen = normalizedScreen;
    _alarmInvertState = false;
    _lastAlarmInvertAt = millis();

    _display.invertDisplay(false);
    _currentScreen = _alarmScreen;
    _showingIpScreen = false;
    _startupIpActive = false;

    if (!_enabled)
    {
      setEnabled(true);
      return;
    }

    render(true);
    return;
  }

  if (!_alarmMode)
  {
    return;
  }

  _alarmMode = false;
  _alarmInvertState = false;
  _display.invertDisplay(false);

  _currentScreen = _screenBeforeAlarm % _screenCount;
  _showingIpScreen = _showingIpBeforeAlarm;
  _startupIpActive = _startupIpActiveBeforeAlarm;
  _startupIpUntil = _startupIpUntilBeforeAlarm;

  if (!_enabledBeforeAlarm)
  {
    setEnabled(false);
    return;
  }

  if (!_enabled)
  {
    setEnabled(true);
    return;
  }

  render(true);
}

bool ScreenController::isAlarmMode() const
{
  return _alarmMode;
}

DisplayScreen *ScreenController::activeScreen()
{
  if (_alarmMode)
  {
    return _screens[_alarmScreen];
  }

  if (_showingIpScreen)
  {
    return &_ipScreen;
  }

  if (_screenCount == 0)
  {
    return nullptr;
  }

  return _screens[_currentScreen];
}

void ScreenController::render(bool force)
{
  if (!_enabled)
  {
    return;
  }

  const unsigned long nowMs = millis();
  if (!force && (nowMs - _lastRenderAt) < RENDER_INTERVAL_MS)
  {
    return;
  }
  _lastRenderAt = nowMs;

  DisplayScreen *screen = activeScreen();
  if (screen == nullptr)
  {
    return;
  }

  _display.clearDisplay();
  screen->draw(_display);
  _display.display();
}

void ScreenController::showMainScreen(uint8_t index)
{
  if (_screenCount == 0)
  {
    return;
  }

  _currentScreen = index % _screenCount;
  _showingIpScreen = false;
  _startupIpActive = false;
  render(true);
}
