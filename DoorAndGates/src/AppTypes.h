#ifndef APP_TYPES_H
#define APP_TYPES_H

#include <Arduino.h>

enum class GateMotorState : uint8_t
{
  Stopped,
  Opening,
  Closing
};

enum class GateState : uint8_t
{
  Unknown,
  Open,
  Closed,
  Opening,
  Closing,
  Stopped
};

enum class MagnetDirection : uint8_t
{
  None,
  Up,
  Down
};

const char *gateStateToText(GateState state);
const char *gateMotorStateToText(GateMotorState state);
const char *magnetDirectionToText(MagnetDirection direction);

#endif
