#include "MatrixDisplay.h"

#include "AppConfig.h"

#include <math.h>

namespace
{
constexpr uint8_t OP_NOOP = 0;
constexpr uint8_t OP_DECODE_MODE = 9;
constexpr uint8_t OP_INTENSITY = 10;
constexpr uint8_t OP_SCAN_LIMIT = 11;
constexpr uint8_t OP_SHUTDOWN = 12;
constexpr uint8_t OP_DISPLAY_TEST = 15;
constexpr uint8_t GLYPH_MINUS = 12;
constexpr uint8_t GLYPH_BLANK = 13;
constexpr uint8_t GLYPH_E = 14;
constexpr uint8_t GLYPH_R = 15;
constexpr uint8_t GLYPH_O = 16;

const uint8_t GLYPHS[17][8] = {
    {B00000000, B00011000, B00100100, B01000010, B01000010, B00100100, B00011000, B00000000},
    {B00000000, B00011100, B00101100, B01001100, B00001100, B00001100, B00001100, B00000000},
    {B00000000, B00111000, B01101100, B00011000, B00110000, B01100000, B01111110, B00000000},
    {B00000000, B00111100, B01100110, B00001100, B00000110, B01100110, B00111100, B00000000},
    {B00000000, B01100000, B01100000, B01101000, B01111110, B00001000, B00001000, B00000000},
    {B00000000, B01111110, B01100000, B01111000, B00000110, B01100110, B00111100, B00000000},
    {B00000000, B00001100, B00111000, B01100000, B01111100, B01100110, B00111100, B00000000},
    {B00000000, B01111110, B00000110, B00001100, B00011000, B00110000, B01100000, B00000000},
    {B00000000, B00111100, B00100100, B00011000, B01100110, B01000010, B00111100, B00000000},
    {B00000000, B00111100, B01100110, B00111110, B00000110, B00011100, B00110000, B00000000},
    {B00010000, B00010000, B00010000, B01111100, B00010000, B00010010, B00011100, B00000000},
    {B01000000, B01000000, B01000000, B01000000, B01111100, B01000100, B01000100, B00000000},
    {B00000000, B00000000, B00000000, B01111110, B00000000, B00000000, B00000000, B00000000},
    {B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000},
    {B00000000, B00111100, B01100010, B01111110, B01100000, B01100010, B00111100, B00000000},
    {B00000000, B00000000, B01011100, B01100010, B01100000, B01100000, B01100000, B00000000},
    {B00000000, B00000000, B00111100, B01000010, B01000010, B01000010, B00111100, B00000000}};

uint8_t reverseBits(uint8_t value)
{
  value = ((value & 0xF0) >> 4) | ((value & 0x0F) << 4);
  value = ((value & 0xCC) >> 2) | ((value & 0x33) << 2);
  value = ((value & 0xAA) >> 1) | ((value & 0x55) << 1);
  return value;
}

uint8_t clampIntensity(uint8_t intensity)
{
  return min<uint8_t>(intensity, Config::MATRIX_MAX_INTENSITY);
}

void invalidateStates(uint8_t states[4][8])
{
  for (uint8_t segment = 0; segment < 4; segment++)
  {
    for (uint8_t row = 0; row < 8; row++)
    {
      states[segment][row] = 0xFF;
    }
  }
}
}

MatrixDisplay::MatrixDisplay(uint8_t dataPin,
                             uint8_t clockPin,
                             uint8_t chipSelectPin,
                             uint8_t segmentCount,
                             uint8_t intensity,
                             uint32_t renderIntervalMs,
                             uint32_t modeIntervalMs)
    : _dataPin(dataPin),
      _clockPin(clockPin),
      _chipSelectPin(chipSelectPin),
      _segmentCount(segmentCount),
      _intensity(clampIntensity(intensity)),
      _renderIntervalMs(renderIntervalMs),
      _modeIntervalMs(modeIntervalMs)
{
}

void MatrixDisplay::begin()
{
  pinMode(_dataPin, OUTPUT);
  pinMode(_clockPin, OUTPUT);
  pinMode(_chipSelectPin, OUTPUT);
  digitalWrite(_dataPin, LOW);
  digitalWrite(_clockPin, LOW);
  digitalWrite(_chipSelectPin, HIGH);

  sendCommandToAll(OP_DISPLAY_TEST, 0);
  sendCommandToAll(OP_SCAN_LIMIT, 7);
  sendCommandToAll(OP_DECODE_MODE, 0);
  _initialized = true;
  setIntensity(_intensity);
  setEnabled(_enabled);

  for (uint8_t row = 0; row < 8; row++)
  {
    sendCommandToAll(row + 1, 0);
  }

  _lastModeChangeAt = millis();
}

void MatrixDisplay::loop(const ClimateSensor &climate)
{
  if (!_enabled)
  {
    return;
  }

  const uint32_t now = millis();
  if ((now - _lastModeChangeAt) >= _modeIntervalMs)
  {
    _lastModeChangeAt = now;
    _showTemperature = !_showTemperature;
  }

  if ((now - _lastRenderAt) < _renderIntervalMs)
  {
    return;
  }

  _lastRenderAt = now;
  render(climate);
}

void MatrixDisplay::setIntensity(uint8_t intensity)
{
  _intensity = clampIntensity(intensity);
  if (_initialized)
  {
    sendCommandToAll(OP_INTENSITY, _intensity);
  }
}

uint8_t MatrixDisplay::intensity() const
{
  return _intensity;
}

void MatrixDisplay::setEnabled(bool enabled)
{
  _enabled = enabled;
  if (!_initialized)
  {
    return;
  }

  sendCommandToAll(OP_SHUTDOWN, _enabled ? 1 : 0);
  if (_enabled)
  {
    invalidateStates(_states);
    _lastRenderAt = 0;
  }
}

bool MatrixDisplay::enabled() const
{
  return _enabled;
}

void MatrixDisplay::render(const ClimateSensor &climate)
{
  if (!climate.valid())
  {
    displayOnSegment(0, GLYPHS[GLYPH_E]);
    displayOnSegment(1, GLYPHS[GLYPH_R]);
    displayOnSegment(2, GLYPHS[GLYPH_R]);
    displayOnSegment(3, GLYPHS[GLYPH_O]);
    return;
  }

  uint8_t first = GLYPH_MINUS;
  uint8_t second = GLYPH_MINUS;
  uint8_t third = GLYPH_MINUS;

  const float value = _showTemperature ? climate.temperature() : climate.humidity();
  splitValue(value, first, second, third);

  displayOnSegment(0, GLYPHS[first]);
  displayOnSegment(1, GLYPHS[second]);
  displayOnSegment(2, GLYPHS[third]);
  displayOnSegment(3, GLYPHS[_showTemperature ? 10 : 11]);
}

void MatrixDisplay::sendCommandToAll(uint8_t opcode, uint8_t data)
{
  beginTransfer();
  for (int8_t segment = _segmentCount - 1; segment >= 0; segment--)
  {
    shiftOut(_dataPin, _clockPin, MSBFIRST, opcode);
    shiftOut(_dataPin, _clockPin, MSBFIRST, data);
  }
  endTransfer();
}

void MatrixDisplay::sendCommandToSegment(uint8_t segment,
                                         uint8_t opcode,
                                         uint8_t data)
{
  if (segment >= _segmentCount)
  {
    return;
  }

  const uint8_t physicalSegment = Config::MATRIX_REVERSE_SEGMENT_ORDER
                                      ? (_segmentCount - 1 - segment)
                                      : segment;

  beginTransfer();
  for (int8_t index = _segmentCount - 1; index >= 0; index--)
  {
    if (index == physicalSegment)
    {
      shiftOut(_dataPin, _clockPin, MSBFIRST, opcode);
      shiftOut(_dataPin, _clockPin, MSBFIRST, data);
    }
    else
    {
      shiftOut(_dataPin, _clockPin, MSBFIRST, OP_NOOP);
      shiftOut(_dataPin, _clockPin, MSBFIRST, 0);
    }
  }
  endTransfer();
}

void MatrixDisplay::displayOnSegment(uint8_t segment, const uint8_t glyph[8])
{
  if (segment >= _segmentCount || segment >= 4)
  {
    return;
  }

  for (uint8_t row = 0; row < 8; row++)
  {
    const uint8_t sourceRow = Config::MATRIX_MIRROR_ROWS ? (7 - row) : row;
    const uint8_t rowData = Config::MATRIX_MIRROR_COLUMNS
                                ? reverseBits(glyph[sourceRow])
                                : glyph[sourceRow];
    if (_states[segment][row] == rowData)
    {
      continue;
    }

    _states[segment][row] = rowData;
    sendCommandToSegment(segment, row + 1, rowData);
  }
}

void MatrixDisplay::beginTransfer()
{
  digitalWrite(_chipSelectPin, LOW);
}

void MatrixDisplay::endTransfer()
{
  digitalWrite(_chipSelectPin, HIGH);
}

void MatrixDisplay::splitValue(float value,
                               uint8_t &first,
                               uint8_t &second,
                               uint8_t &third)
{
  first = GLYPH_BLANK;
  second = GLYPH_BLANK;
  third = GLYPH_BLANK;

  int rounded = static_cast<int>(roundf(value));
  rounded = constrain(rounded, -99, 999);

  if (rounded < 0)
  {
    const int absolute = -rounded;
    first = absolute % 10;
    if (absolute < 10)
    {
      second = GLYPH_MINUS;
      return;
    }

    second = (absolute / 10) % 10;
    third = GLYPH_MINUS;
    return;
  }

  first = rounded % 10;
  if (rounded >= 10)
  {
    second = (rounded / 10) % 10;
  }
  if (rounded >= 100)
  {
    third = (rounded / 100) % 10;
  }
}
