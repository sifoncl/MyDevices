#include "DisplayController.h"

DisplayController::DisplayController(uint8_t width,
                                     uint8_t height,
                                     TwoWire &wire,
                                     uint8_t address,
                                     unsigned long renderIntervalMs,
                                     unsigned long ipScreenMs)
    : _display(width, height, &wire, -1),
      _address(address),
      _renderIntervalMs(renderIntervalMs),
      _ipScreenMs(ipScreenMs)
{
}

void DisplayController::begin()
{
  _available = _display.begin(SSD1306_SWITCHCAPVCC, _address);
  if (!_available)
  {
    Serial.println("SSD1306 not found");
    return;
  }

  _display.clearDisplay();
  setBrightness(_brightness);
  _display.display();
}

void DisplayController::loop(const ClimateController &climate, const NetworkManager &network)
{
  if (!_available || (millis() - _lastRenderAt) < _renderIntervalMs)
  {
    return;
  }
  _lastRenderAt = millis();

  if (_ipScreenUntil != 0 && static_cast<long>(millis() - _ipScreenUntil) < 0)
  {
    drawIp(network);
    return;
  }

  _ipScreenUntil = 0;
  drawClimate(climate);
}

void DisplayController::showIp(const NetworkManager &network)
{
  if (!_available)
  {
    return;
  }

  _ipScreenUntil = millis() + _ipScreenMs;
  drawIp(network);
  _lastRenderAt = millis();
}

bool DisplayController::available() const
{
  return _available;
}

void DisplayController::setBrightness(uint8_t brightness)
{
  _brightness = brightness;
  if (!_available)
  {
    return;
  }

  _display.ssd1306_command(SSD1306_SETCONTRAST);
  _display.ssd1306_command(_brightness);
}

uint8_t DisplayController::brightness() const
{
  return _brightness;
}

void DisplayController::drawCenteredText(const String &text, uint8_t textSize, int16_t y)
{
  int16_t x1 = 0;
  int16_t y1 = 0;
  uint16_t width = 0;
  uint16_t height = 0;

  _display.setTextSize(textSize);
  _display.getTextBounds(text, 0, y, &x1, &y1, &width, &height);
  _display.setCursor((_display.width() - static_cast<int16_t>(width)) / 2 - x1, y);
  _display.println(text);
}

void DisplayController::drawClimate(const ClimateController &climate)
{
  _display.clearDisplay();
  _display.setTextColor(WHITE);
  _display.setTextSize(1);
  _display.setCursor(0, 0);
  _display.print("DoorAndGates");
  _display.drawLine(0, 10, _display.width() - 1, 10, WHITE);

  if (climate.valid())
  {
    _display.setTextSize(2);
    _display.setCursor(0, 18);
    _display.print("T: ");
    _display.print(climate.temperature(), 1);
    _display.print(" C");

    _display.setCursor(0, 42);
    _display.print("H: ");
    _display.print(climate.humidity(), 1);
    _display.print(" %");
  }
  else
  {
    drawCenteredText("SHT31 unavailable", 1, 30);
  }

  _display.display();
}

void DisplayController::drawIp(const NetworkManager &network)
{
  _display.clearDisplay();
  _display.setTextColor(WHITE);
  drawCenteredText(network.connected() ? "WiFi connected" : "Setup access point", 1, 0);
  drawCenteredText(network.provisioningMode() ? network.setupApSsid() : "IP address", 1, 18);

  // Text size 1 keeps IPv4 addresses on one line on a 128 px display.
  drawCenteredText(network.address().toString(), 1, 39);
  _display.display();
}
