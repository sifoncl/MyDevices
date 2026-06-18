#ifndef GATE_CONTROLLER_H
#define GATE_CONTROLLER_H

#include <Arduino.h>
#include <MagneticPresenceSensor.h>

#include "AppTypes.h"

class GateController
{
public:
  GateController(uint8_t openRelayPin,
                 uint8_t closeRelayPin,
                 uint8_t wicketRelayPin,
                 uint8_t lightRelayPin,
                 uint8_t relayActiveLevel,
                 unsigned long switchGuardMs,
                 unsigned long wicketPulseMs);

  void begin();
  void loop();

  void commandOpen();
  void commandClose();
  void commandStop();
  void triggerWicket();
  void setLight(bool enabled);
  void toggleLight();
  void setWicketOpen(bool open);

  void processMagnetStates(const MagneticPresenceSensorState &upper,
                           const MagneticPresenceSensorState &lower);

  GateState state() const;
  GateMotorState motorState() const;
  bool wicketOpen() const;
  bool wicketRelayActive() const;
  bool lightOn() const;
  uint32_t stateRevision() const;

private:
  uint8_t _openRelayPin;
  uint8_t _closeRelayPin;
  uint8_t _wicketRelayPin;
  uint8_t _lightRelayPin;
  uint8_t _relayActiveLevel;
  unsigned long _switchGuardMs;
  unsigned long _wicketPulseMs;

  GateState _state = GateState::Unknown;
  GateMotorState _motorState = GateMotorState::Stopped;
  bool _wicketOpen = false;
  bool _wicketRelayActive = false;
  bool _lightOn = false;
  unsigned long _wicketPulseUntil = 0;
  uint32_t _stateRevision = 0;

  uint8_t inactiveRelayLevel() const;
  void writeRelay(uint8_t pin, bool enabled);
  void allMotorRelaysOff();
  void setGateState(GateState state);
  void stopAtLimit(GateState limitState);
  static MagnetDirection directionOf(int8_t direction);
};

#endif
