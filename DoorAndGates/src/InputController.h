#ifndef INPUT_CONTROLLER_H
#define INPUT_CONTROLLER_H

#include "DebouncedInput.h"
#include "GateController.h"

class InputController
{
public:
  explicit InputController(GateController &gate);

  void begin();
  void loop();

private:
  GateController &_gate;

  DebouncedInput _wicketReed;
  DebouncedInput _radioOpenButton;
  DebouncedInput _radioCloseButton;
  DebouncedInput _radioWicketButton;
  DebouncedInput _radioLightButton;

  void radioButtonsLoop();
  void wicketLoop();
};

#endif
