#ifndef DISPLAY_CONTROLLER_H
#define DISPLAY_CONTROLLER_H

#include <Adafruit_SSD1306.h>

#include "ClimateController.h"
#include "NetworkManager.h"

class DisplayController
{
public:
  DisplayController(uint8_t width,
                    uint8_t height,
                    TwoWire &wire,
                    uint8_t address,
                    unsigned long renderIntervalMs,
                    unsigned long ipScreenMs);

  void begin();
  void loop(const ClimateController &climate, const NetworkManager &network);
  void showIp(const NetworkManager &network);
  bool available() const;
  void setBrightness(uint8_t brightness);
  uint8_t brightness() const;

private:
  Adafruit_SSD1306 _display;
  uint8_t _address;
  unsigned long _renderIntervalMs;
  unsigned long _ipScreenMs;
  unsigned long _lastRenderAt = 0;
  unsigned long _ipScreenUntil = 0;
  bool _available = false;
  uint8_t _brightness = 255;

  void drawCenteredText(const String &text, uint8_t textSize, int16_t y);
  void drawClimate(const ClimateController &climate);
  void drawIp(const NetworkManager &network);
};

#endif
