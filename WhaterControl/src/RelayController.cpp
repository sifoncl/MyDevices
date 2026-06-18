#include "RelayController.h"

RelayController::RelayController(const uint8_t *pins, uint8_t count, uint8_t activeLevel)
    : _pins(pins),
      _count(count > MAX_RELAYS ? MAX_RELAYS : count),
      _activeLevel(activeLevel)
{
}

void RelayController::begin()
{
  for (uint8_t i = 0; i < _count; i++)
  {
    digitalWrite(_pins[i], inactiveLevel());
    pinMode(_pins[i], OUTPUT);
    apply(i);
  }
}

void RelayController::set(uint8_t index, bool enabled)
{
  if (index >= _count || _states[index] == enabled)
  {
    return;
  }

  _states[index] = enabled;
  _revision++;
  apply(index);
}

void RelayController::toggle(uint8_t index)
{
  if (index >= _count)
  {
    return;
  }
  set(index, !_states[index]);
}

bool RelayController::state(uint8_t index) const
{
  if (index >= _count)
  {
    return false;
  }
  return _states[index];
}

uint8_t RelayController::count() const
{
  return _count;
}

uint32_t RelayController::revision() const
{
  return _revision;
}

uint8_t RelayController::inactiveLevel() const
{
  return _activeLevel == HIGH ? LOW : HIGH;
}

void RelayController::apply(uint8_t index)
{
  if (index >= _count)
  {
    return;
  }
  digitalWrite(_pins[index], _states[index] ? _activeLevel : inactiveLevel());
}
