#include "GateController.h"

GateController::GateController(uint8_t openRelayPin,
                               uint8_t closeRelayPin,
                               uint8_t wicketRelayPin,
                               uint8_t lightRelayPin,
                               uint8_t relayActiveLevel,
                               unsigned long switchGuardMs,
                               unsigned long wicketPulseMs)
    : _openRelayPin(openRelayPin),
      _closeRelayPin(closeRelayPin),
      _wicketRelayPin(wicketRelayPin),
      _lightRelayPin(lightRelayPin),
      _relayActiveLevel(relayActiveLevel),
      _switchGuardMs(switchGuardMs),
      _wicketPulseMs(wicketPulseMs)
{
}

void GateController::begin()
{
  pinMode(_openRelayPin, OUTPUT);
  pinMode(_closeRelayPin, OUTPUT);
  pinMode(_wicketRelayPin, OUTPUT);
  pinMode(_lightRelayPin, OUTPUT);

  allMotorRelaysOff();
  writeRelay(_wicketRelayPin, false);
  writeRelay(_lightRelayPin, false);
}

void GateController::loop()
{
  if (_wicketRelayActive && static_cast<long>(millis() - _wicketPulseUntil) >= 0)
  {
    writeRelay(_wicketRelayPin, false);
    _wicketRelayActive = false;
    _stateRevision++;
  }
}

void GateController::commandOpen()
{
  allMotorRelaysOff();
  delay(_switchGuardMs);
  writeRelay(_closeRelayPin, false);
  writeRelay(_openRelayPin, true);
  _motorState = GateMotorState::Opening;
  setGateState(GateState::Opening);
}

void GateController::commandClose()
{
  allMotorRelaysOff();
  delay(_switchGuardMs);
  writeRelay(_openRelayPin, false);
  writeRelay(_closeRelayPin, true);
  _motorState = GateMotorState::Closing;
  setGateState(GateState::Closing);
}

void GateController::commandStop()
{
  allMotorRelaysOff();

  if (_state != GateState::Open && _state != GateState::Closed)
  {
    setGateState(GateState::Stopped);
  }
}

void GateController::triggerWicket()
{
  writeRelay(_wicketRelayPin, true);
  _wicketRelayActive = true;
  _wicketPulseUntil = millis() + _wicketPulseMs;
  _stateRevision++;
}

void GateController::setLight(bool enabled)
{
  if (_lightOn == enabled)
  {
    return;
  }

  _lightOn = enabled;
  writeRelay(_lightRelayPin, enabled);
  _stateRevision++;
}

void GateController::toggleLight()
{
  setLight(!_lightOn);
}

void GateController::setWicketOpen(bool open)
{
  if (_wicketOpen == open)
  {
    return;
  }

  _wicketOpen = open;
  _stateRevision++;
}

void GateController::processMagnetStates(const MagneticPresenceSensorState &upper,
                                         const MagneticPresenceSensorState &lower)
{
  const MagnetDirection upperDirection = directionOf(upper.direction);
  const MagnetDirection lowerDirection = directionOf(lower.direction);

  if ((upper.justMotionStarted || upper.moving) && upper.active)
  {
    if (upperDirection == MagnetDirection::Up)
    {
      setGateState(GateState::Opening);
    }
    else if (upperDirection == MagnetDirection::Down)
    {
      setGateState(GateState::Closing);
    }
  }

  if ((lower.justMotionStarted || lower.moving) && lower.active)
  {
    if (lowerDirection == MagnetDirection::Up)
    {
      setGateState(GateState::Opening);
    }
    else if (lowerDirection == MagnetDirection::Down)
    {
      setGateState(GateState::Closing);
    }
  }

  const MagnetDirection upperSettled = directionOf(upper.settledDirection);
  const MagnetDirection lowerSettled = directionOf(lower.settledDirection);

  if (upper.justSettled && upperSettled == MagnetDirection::Up)
  {
    stopAtLimit(GateState::Open);
    return;
  }

  if (lower.justSettled && lowerSettled == MagnetDirection::Down)
  {
    stopAtLimit(GateState::Closed);
    return;
  }

  if (upper.justSettled && upperSettled == MagnetDirection::Down)
  {
    setGateState(GateState::Closing);
  }
  else if (lower.justSettled && lowerSettled == MagnetDirection::Up)
  {
    setGateState(GateState::Opening);
  }

  if (_motorState == GateMotorState::Stopped && _state == GateState::Unknown)
  {
    if (upper.active && !lower.active)
    {
      setGateState(GateState::Open);
    }
    else if (lower.active && !upper.active)
    {
      setGateState(GateState::Closed);
    }
  }
}

GateState GateController::state() const
{
  return _state;
}

GateMotorState GateController::motorState() const
{
  return _motorState;
}

bool GateController::wicketOpen() const
{
  return _wicketOpen;
}

bool GateController::wicketRelayActive() const
{
  return _wicketRelayActive;
}

bool GateController::lightOn() const
{
  return _lightOn;
}

uint32_t GateController::stateRevision() const
{
  return _stateRevision;
}

uint8_t GateController::inactiveRelayLevel() const
{
  return _relayActiveLevel == HIGH ? LOW : HIGH;
}

void GateController::writeRelay(uint8_t pin, bool enabled)
{
  digitalWrite(pin, enabled ? _relayActiveLevel : inactiveRelayLevel());
}

void GateController::allMotorRelaysOff()
{
  writeRelay(_openRelayPin, false);
  writeRelay(_closeRelayPin, false);
  _motorState = GateMotorState::Stopped;
}

void GateController::setGateState(GateState state)
{
  if (_state == state)
  {
    return;
  }

  _state = state;
  _stateRevision++;
}

void GateController::stopAtLimit(GateState limitState)
{
  allMotorRelaysOff();
  setGateState(limitState);
}

MagnetDirection GateController::directionOf(int8_t direction)
{
  if (direction > 0)
  {
    return MagnetDirection::Up;
  }
  if (direction < 0)
  {
    return MagnetDirection::Down;
  }
  return MagnetDirection::None;
}
