#ifndef PC_POWER_CONTROLLER_H
#define PC_POWER_CONTROLLER_H

#include <Arduino.h>

#include "PcPowerSettings.h"

enum class PcPowerAction
{
  Idle,
  TurningOn,
  TurningOff,
  Failed
};

class PcPowerController
{
public:
  PcPowerController(uint8_t statusPin,
                    uint8_t buttonPin,
                    uint8_t statusInputMode,
                    PcPowerSettings &settings);

  void begin();
  void loop();
  void applySettings();

  void commandTurnOn();
  void commandTurnOff();
  void toggle();
  void pulseButton();
  void cancelCommand();

  bool isOn() const;
  bool rawOn() const;
  bool desiredOn() const;
  bool commandActive() const;
  bool buttonActive() const;
  bool statusChanging() const;
  PcPowerAction action() const;
  const char *actionText() const;
  uint8_t attempts() const;
  uint32_t statusAgeMs() const;
  uint32_t revision() const;
  const String &lastMessage() const;

private:
  uint8_t _statusPin;
  uint8_t _buttonPin;
  uint8_t _statusInputMode;
  PcPowerSettings &_settings;

  bool _rawOn = false;
  bool _candidateOn = false;
  bool _stableOn = false;
  bool _desiredOn = false;
  uint32_t _candidateChangedAt = 0;
  uint32_t _lastStableChangedAt = 0;

  PcPowerAction _action = PcPowerAction::Idle;
  bool _pulseActive = false;
  uint32_t _pulseStartedAt = 0;
  uint32_t _nextAttemptAt = 0;
  uint8_t _attempts = 0;
  uint32_t _revision = 0;
  String _lastMessage = "idle";

  bool readRawOn() const;
  void readStatus(uint32_t now);
  void startCommand(bool desiredOn);
  void startPulse(uint32_t now);
  void releaseButton();
  void updatePulse(uint32_t now);
  void completeCommand(const char *message);
  void failCommand();
  uint8_t maxAttemptsForCurrentAction() const;
  uint32_t retryMsForCurrentAction() const;
  void bumpRevision();
  static bool due(uint32_t now, uint32_t scheduledAt);
};

#endif
