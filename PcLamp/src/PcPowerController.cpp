#include "PcPowerController.h"

PcPowerController::PcPowerController(uint8_t statusPin,
                                     uint8_t buttonPin,
                                     uint8_t statusInputMode,
                                     PcPowerSettings &settings)
    : _statusPin(statusPin),
      _buttonPin(buttonPin),
      _statusInputMode(statusInputMode),
      _settings(settings)
{
}

void PcPowerController::begin()
{
  pinMode(_statusPin, _statusInputMode);
  releaseButton();

  const uint32_t now = millis();
  _rawOn = readRawOn();
  _candidateOn = _rawOn;
  _stableOn = _rawOn;
  _desiredOn = _stableOn;
  _candidateChangedAt = now;
  _lastStableChangedAt = now;
  bumpRevision();

  Serial.print("PC initial state: ");
  Serial.println(_stableOn ? "on" : "off");
}

void PcPowerController::loop()
{
  const uint32_t now = millis();
  readStatus(now);
  updatePulse(now);

  if (_action == PcPowerAction::Failed)
  {
    if (_stableOn == _desiredOn)
    {
      completeCommand("target_reached_after_fail");
    }
    return;
  }

  if (!commandActive())
  {
    return;
  }

  if (_stableOn == _desiredOn)
  {
    completeCommand(_stableOn ? "pc_on" : "pc_off");
    return;
  }

  if (_attempts >= maxAttemptsForCurrentAction())
  {
    failCommand();
    return;
  }

  if (!_pulseActive && due(now, _nextAttemptAt))
  {
    _attempts++;
    _nextAttemptAt = now + retryMsForCurrentAction();
    startPulse(now);

    Serial.print(_desiredOn ? "PC start attempt " : "PC shutdown attempt ");
    Serial.println(_attempts);
  }
}

void PcPowerController::applySettings()
{
  pinMode(_statusPin, _statusInputMode);
  releaseButton();

  const uint32_t now = millis();
  _rawOn = readRawOn();
  _candidateOn = _rawOn;
  _stableOn = _rawOn;
  _desiredOn = _stableOn;
  _candidateChangedAt = now;
  _lastStableChangedAt = now;
  _action = PcPowerAction::Idle;
  _attempts = 0;
  _lastMessage = "settings_applied";
  bumpRevision();
}

void PcPowerController::commandTurnOn()
{
  startCommand(true);
}

void PcPowerController::commandTurnOff()
{
  startCommand(false);
}

void PcPowerController::toggle()
{
  startCommand(!_stableOn);
}

void PcPowerController::pulseButton()
{
  if (!_pulseActive)
  {
    startPulse(millis());
    _lastMessage = "manual_pulse";
    bumpRevision();
  }
}

void PcPowerController::cancelCommand()
{
  releaseButton();
  _action = PcPowerAction::Idle;
  _desiredOn = _stableOn;
  _attempts = 0;
  _lastMessage = "cancelled";
  bumpRevision();
}

bool PcPowerController::isOn() const
{
  return _stableOn;
}

bool PcPowerController::rawOn() const
{
  return _rawOn;
}

bool PcPowerController::desiredOn() const
{
  return _desiredOn;
}

bool PcPowerController::commandActive() const
{
  return _action == PcPowerAction::TurningOn ||
         _action == PcPowerAction::TurningOff;
}

bool PcPowerController::buttonActive() const
{
  return _pulseActive;
}

bool PcPowerController::statusChanging() const
{
  return _rawOn != _stableOn;
}

PcPowerAction PcPowerController::action() const
{
  return _action;
}

const char *PcPowerController::actionText() const
{
  switch (_action)
  {
  case PcPowerAction::TurningOn:
    return "turning_on";
  case PcPowerAction::TurningOff:
    return "turning_off";
  case PcPowerAction::Failed:
    return "failed";
  case PcPowerAction::Idle:
  default:
    return "idle";
  }
}

uint8_t PcPowerController::attempts() const
{
  return _attempts;
}

uint32_t PcPowerController::statusAgeMs() const
{
  return millis() - _lastStableChangedAt;
}

uint32_t PcPowerController::revision() const
{
  return _revision;
}

const String &PcPowerController::lastMessage() const
{
  return _lastMessage;
}

bool PcPowerController::readRawOn() const
{
  const bool levelHigh = digitalRead(_statusPin) == HIGH;
  return _settings.statusActiveHigh() ? levelHigh : !levelHigh;
}

void PcPowerController::readStatus(uint32_t now)
{
  const bool raw = readRawOn();
  if (raw != _rawOn)
  {
    _rawOn = raw;
    bumpRevision();
  }

  if (raw != _candidateOn)
  {
    _candidateOn = raw;
    _candidateChangedAt = now;
    return;
  }

  if (_stableOn != raw && (now - _candidateChangedAt) >= _settings.statusDebounceMs())
  {
    _stableOn = raw;
    _lastStableChangedAt = now;
    _lastMessage = _stableOn ? "status_on" : "status_off";
    bumpRevision();

    Serial.print("PC stable state: ");
    Serial.println(_stableOn ? "on" : "off");
  }
}

void PcPowerController::startCommand(bool desiredOn)
{
  if (_stableOn == desiredOn && !commandActive())
  {
    _desiredOn = desiredOn;
    _action = PcPowerAction::Idle;
    _attempts = 0;
    _lastMessage = desiredOn ? "already_on" : "already_off";
    bumpRevision();
    return;
  }

  _desiredOn = desiredOn;
  _action = desiredOn ? PcPowerAction::TurningOn : PcPowerAction::TurningOff;
  _attempts = 0;
  _nextAttemptAt = millis();
  _lastMessage = desiredOn ? "turn_on_requested" : "turn_off_requested";
  bumpRevision();

  Serial.println(desiredOn ? "PC turn on requested" : "PC turn off requested");
}

void PcPowerController::startPulse(uint32_t now)
{
  const uint8_t activeLevel = _settings.buttonActiveHigh() ? HIGH : LOW;
  digitalWrite(_buttonPin, activeLevel);
  pinMode(_buttonPin, OUTPUT);
  _pulseActive = true;
  _pulseStartedAt = now;
  bumpRevision();
}

void PcPowerController::releaseButton()
{
  const uint8_t idleLevel = _settings.buttonActiveHigh() ? LOW : HIGH;
  digitalWrite(_buttonPin, idleLevel);
  pinMode(_buttonPin, _settings.buttonActiveHigh() ? INPUT_PULLDOWN : INPUT_PULLUP);
  _pulseActive = false;
}

void PcPowerController::updatePulse(uint32_t now)
{
  if (_pulseActive && (now - _pulseStartedAt) >= _settings.buttonPulseMs())
  {
    releaseButton();
    bumpRevision();
  }
}

void PcPowerController::completeCommand(const char *message)
{
  _action = PcPowerAction::Idle;
  _attempts = 0;
  _lastMessage = message;
  bumpRevision();
}

void PcPowerController::failCommand()
{
  _action = PcPowerAction::Failed;
  _lastMessage = _desiredOn ? "turn_on_failed" : "turn_off_failed";
  bumpRevision();

  Serial.print("PC command failed: ");
  Serial.println(_lastMessage);
}

uint8_t PcPowerController::maxAttemptsForCurrentAction() const
{
  return _desiredOn ? _settings.maxStartAttempts() : _settings.maxShutdownAttempts();
}

uint32_t PcPowerController::retryMsForCurrentAction() const
{
  return _desiredOn ? _settings.startRetryMs() : _settings.shutdownRetryMs();
}

void PcPowerController::bumpRevision()
{
  _revision++;
}

bool PcPowerController::due(uint32_t now, uint32_t scheduledAt)
{
  return static_cast<int32_t>(now - scheduledAt) >= 0;
}
