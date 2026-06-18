#ifndef MATRIX_DISPLAY_H
#define MATRIX_DISPLAY_H

#include <Arduino.h>

#include "ClimateSensor.h"

class MatrixDisplay
{
public:
  MatrixDisplay(uint8_t dataPin,
                uint8_t clockPin,
                uint8_t chipSelectPin,
                uint8_t segmentCount,
                uint8_t intensity,
                uint32_t renderIntervalMs,
                uint32_t modeIntervalMs);

  void begin();
  void loop(const ClimateSensor &climate);
  void setIntensity(uint8_t intensity);
  uint8_t intensity() const;
  void setEnabled(bool enabled);
  bool enabled() const;

private:
  uint8_t _dataPin;
  uint8_t _clockPin;
  uint8_t _chipSelectPin;
  uint8_t _segmentCount;
  uint8_t _intensity;
  uint32_t _renderIntervalMs;
  uint32_t _modeIntervalMs;
  uint32_t _lastRenderAt = 0;
  uint32_t _lastModeChangeAt = 0;
  bool _showTemperature = true;
  bool _initialized = false;
  bool _enabled = true;
  uint8_t _states[4][8] = {};

  void render(const ClimateSensor &climate);
  void sendCommandToAll(uint8_t opcode, uint8_t data);
  void sendCommandToSegment(uint8_t segment, uint8_t opcode, uint8_t data);
  void displayOnSegment(uint8_t segment, const uint8_t glyph[8]);
  void beginTransfer();
  void endTransfer();
  static void splitValue(float value,
                         uint8_t &first,
                         uint8_t &second,
                         uint8_t &third);
};

#endif
