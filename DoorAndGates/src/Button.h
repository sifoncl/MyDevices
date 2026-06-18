// Button.h
#ifndef Button_h
#define Button_h

#include <Arduino.h>

class Button {
  private:
    uint8_t _pin;                 // пин кнопки
    uint8_t _activeLevel;         // уровень нажатия (LOW / HIGH)
    unsigned long _debounceTime;  // время антидребезга (мс)

    bool _lastRawState;           // последнее считанное значение (с учётом _activeLevel)
    bool _state;                  // текущее стабильное состояние (true – нажата)
    unsigned long _lastChangeTime; // время последнего изменения сырого состояния
    unsigned long _pressStartTime; // время начала стабильного нажатия

    // Указатели на функции обратного вызова
    void (*_onPressCallback)();    // вызывается при нажатии
    void (*_onReleaseCallback)();  // вызывается при отпускании

  public:
    /**
     * Конструктор
     * @param pin         номер пина
     * @param activeLevel уровень, соответствующий нажатой кнопке (LOW или HIGH).
     *                    По умолчанию LOW (подтяжка к VCC, например INPUT_PULLUP).
     * @param debounceMs  время антидребезга в миллисекундах (по умолчанию 50)
     * @param onPress     указатель на функцию, вызываемую при нажатии (может быть nullptr)
     * @param onRelease   указатель на функцию, вызываемую при отпускании (может быть nullptr)
     */
    Button(uint8_t pin,
           uint8_t activeLevel = LOW,
           unsigned long debounceMs = 50,
           void (*onPress)() = nullptr,
           void (*onRelease)() = nullptr)
      : _pin(pin),
        _activeLevel(activeLevel),
        _debounceTime(debounceMs),
        _onPressCallback(onPress),
        _onReleaseCallback(onRelease) {

      bool raw = (digitalRead(_pin) == _activeLevel);
      _lastRawState = raw;
      _state = raw;
      _lastChangeTime = millis();
      _pressStartTime = _state ? _lastChangeTime : 0;
    }

    /**
     * Установить функцию, вызываемую при нажатии.
     */
    void setOnPress(void (*callback)()) {
      _onPressCallback = callback;
    }

    /**
     * Установить функцию, вызываемую при отпускании.
     */
    void setOnRelease(void (*callback)()) {
      _onReleaseCallback = callback;
    }

    /**
     * Должен вызываться в каждом цикле loop() для обновления состояния кнопки.
     * При обнаружении стабильного нажатия/отпускания автоматически вызывает
     * соответствующие callback-функции (если они заданы).
     */
    void update() {
      bool raw = (digitalRead(_pin) == _activeLevel); // сырое состояние с учётом активного уровня

      // Если состояние изменилось – запоминаем время
      if (raw != _lastRawState) {
        _lastChangeTime = millis();
        _lastRawState = raw;
      }

      // Если состояние держится стабильно дольше задержки – принимаем его
      if ((millis() - _lastChangeTime) >= _debounceTime) {
        // Если стабильное состояние отличается от текущего зафиксированного
        if (raw != _state) {
          _state = raw;

          if (_state) {
            // Нажатие стабилизировалось
            _pressStartTime = millis();
            if (_onPressCallback) {
              _onPressCallback();   // вызываем пользовательский обработчик
            }
          } else {
            // Отпускание стабилизировалось
            if (_onReleaseCallback) {
              _onReleaseCallback(); // вызываем пользовательский обработчик
            }
          }
        }
      }
    }

    // ---- Методы ручного опроса состояния (необязательны при использовании колбэков) ----

    /**
     * @return true, если кнопка нажата (стабильное состояние)
     */
    bool isPressed() const {
      return _state;
    }

    /**
     * @return true, если кнопка отпущена (стабильное состояние)
     */
    bool isReleased() const {
      return !_state;
    }

    /**
     * Проверяет, удерживается ли кнопка дольше указанного времени.
     * @param ms время в миллисекундах
     * @return true, если кнопка непрерывно нажата не менее ms миллисекунд
     */
    bool pressedFor(unsigned long ms) const {
      if (!_state) return false;
      return (millis() - _pressStartTime) >= ms;
    }

    /**
     * @return длительность текущего стабильного нажатия (мс), или 0 если кнопка отпущена
     */
    unsigned long pressDuration() const {
      return _state ? (millis() - _pressStartTime) : 0;
    }
};

#endif
