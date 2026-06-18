#include "InputController.h"

#include "AppConfig.h"

InputController::InputController(GateController &gate)
    : _gate(gate),
      _wicketReed(WICKET_REED_PIN, WICKET_REED_OPEN_LEVEL, BUTTON_DEBOUNCE_MS),
      _radioOpenButton(RADIO_GATE_OPEN_BUTTON_PIN, RADIO_BUTTON_ACTIVE_LEVEL, RADIO_BUTTON_DEBOUNCE_MS),
      _radioCloseButton(RADIO_GATE_CLOSE_BUTTON_PIN, RADIO_BUTTON_ACTIVE_LEVEL, RADIO_BUTTON_DEBOUNCE_MS),
      _radioWicketButton(RADIO_WICKET_BUTTON_PIN, RADIO_BUTTON_ACTIVE_LEVEL, RADIO_BUTTON_DEBOUNCE_MS),
      _radioLightButton(RADIO_LIGHT_BUTTON_PIN, RADIO_BUTTON_ACTIVE_LEVEL, RADIO_BUTTON_DEBOUNCE_MS)
{
}

void InputController::begin()
{
  _wicketReed.begin(INPUT_PULLUP);
  _radioOpenButton.begin(RADIO_BUTTON_INPUT_MODE);
  _radioCloseButton.begin(RADIO_BUTTON_INPUT_MODE);
  _radioWicketButton.begin(RADIO_BUTTON_INPUT_MODE);
  _radioLightButton.begin(RADIO_BUTTON_INPUT_MODE);
}

void InputController::loop()
{
  radioButtonsLoop();
  wicketLoop();
}

void InputController::radioButtonsLoop()
{
  _radioOpenButton.update();
  _radioCloseButton.update();
  _radioWicketButton.update();
  _radioLightButton.update();

  const bool openCommand = _radioOpenButton.justPressed();
  const bool closeCommand = _radioCloseButton.justPressed();

  if (openCommand && closeCommand)
  {
    _gate.commandStop();
  }
  else if (openCommand)
  {
    if (_gate.motorState() == GateMotorState::Opening)
    {
      _gate.commandStop();
    }
    else if (_gate.state() != GateState::Open)
    {
      _gate.commandOpen();
    }
  }
  else if (closeCommand)
  {
    if (_gate.motorState() == GateMotorState::Closing)
    {
      _gate.commandStop();
    }
    else if (_gate.state() != GateState::Closed)
    {
      _gate.commandClose();
    }
  }

  if (_radioWicketButton.justPressed())
  {
    _gate.triggerWicket();
  }
  if (_radioLightButton.justPressed())
  {
    _gate.toggleLight();
  }
}

void InputController::wicketLoop()
{
  _wicketReed.update();
  _gate.setWicketOpen(_wicketReed.isPressed());
}
