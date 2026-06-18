// Gate.h
#ifndef Gate_h
#define Gate_h

#include <Arduino.h>
#include <ArduinoHA.h>
class Gate
{

private:
    HACover::CoverState currentState;
    HACover::CoverCommand targetCommnad;

    uint8_t _openRelayPin;
    uint8_t _closeRelayPin;

    uint8_t _openTriggerPin;
    uint8_t _closeTriggerPin;

    uint8_t _openSensorPin;
    uint8_t _closeSensorPin;

    // bool openRelayStatus;
    // bool closeRelayStatus;

    void turnOnOpenRelay()
    {
        digitalWrite(_closeRelayPin, false);
        digitalWrite(_openRelayPin, true);
    }

    void turnOnCloseRelay()
    {
        digitalWrite(_openRelayPin, false);
        digitalWrite(_closeRelayPin, true);
    }

    void turnOffOpenRelay()
    {
        digitalWrite(_openRelayPin, false);
    }

    void turnOffCloseRelay()
    {
        digitalWrite(_closeRelayPin, false);
    }

    void initPins()
    {
        pinMode(_openRelayPin, OUTPUT);
        pinMode(_closeRelayPin, OUTPUT);

        pinMode(_openTriggerPin, INPUT);
        pinMode(_closeTriggerPin, INPUT);

        pinMode(_openSensorPin, INPUT);
        pinMode(_closeSensorPin, INPUT);
        // Убедимся, что все выключены при старте
        digitalWrite(_openRelayPin, false);
        digitalWrite(_closeRelayPin, false);
    }

    bool readCloseSensor()
    {
        return digitalRead(_closeSensorPin);
    }

    bool readOpenSensor()
    {
        return digitalRead(_openSensorPin);
    }

    void initState()
    {
        targetCommnad = HACover::CommandStop;
        if (readCloseSensor && readOpenSensor)
        {
            currentState = HACover::CoverState::StateClosed;
        }
        if (!readCloseSensor && readOpenSensor)
        {
            currentState = HACover::CoverState::StateClosing;
        }
        if (!readCloseSensor && !readOpenSensor)
        {
            currentState = HACover::CoverState::StateOpen;
        }
    }

    void gateLoop()
    {
        switch (targetCommnad)
        {
        case HACover::CommandStop:
            /* code */
            break;
        case HACover::CommandOpen:
            /* code */
            break;
        case HACover::CommandClose:
            /* code */
            break;
        default:
            break;
        }
    }

    Gate(uint8_t pin,
         uint8_t activeLevel = LOW,
         unsigned long debounceMs = 50,
         void (*onPress)() = nullptr,
         void (*onRelease)() = nullptr)
        : _pin(pin),
          _activeLevel(activeLevel),
          _debounceTime(debounceMs),
          _onPressCallback(onPress),
          _onReleaseCallback(onRelease)
    {

        initPins;
        initState;
    }
}
#endif