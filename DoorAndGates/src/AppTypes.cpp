#include "AppTypes.h"

const char *gateStateToText(GateState state)
{
  switch (state)
  {
  case GateState::Open:
    return "open";
  case GateState::Closed:
    return "closed";
  case GateState::Opening:
    return "opening";
  case GateState::Closing:
    return "closing";
  case GateState::Stopped:
    return "stopped";
  case GateState::Unknown:
  default:
    return "unknown";
  }
}

const char *gateMotorStateToText(GateMotorState state)
{
  switch (state)
  {
  case GateMotorState::Opening:
    return "opening";
  case GateMotorState::Closing:
    return "closing";
  case GateMotorState::Stopped:
  default:
    return "stopped";
  }
}

const char *magnetDirectionToText(MagnetDirection direction)
{
  switch (direction)
  {
  case MagnetDirection::Up:
    return "up";
  case MagnetDirection::Down:
    return "down";
  case MagnetDirection::None:
  default:
    return "none";
  }
}
