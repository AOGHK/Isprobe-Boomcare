
#pragma once

#include <Arduino.h>

class MultiButton {
public:
  MultiButton()
    : _lastTransition(millis()), _state(StateIdle), _new(false){};

  void update(bool pressed) {
    _new = false;

    if (!pressed && _state == StateIdle) {
      return;
    }

    unsigned int now = millis();
    int diff = now - _lastTransition;

    State next = StateIdle;
    switch (_state) {
      case StateIdle: next = _checkIdle(pressed, diff); break;
      case StateDebounce: next = _checkDebounce(pressed, diff); break;
      case StatePressed: next = _checkPressed(pressed, diff); break;
      case StateClickUp: next = _checkClickUp(pressed, diff); break;
      case StateClickIdle: next = _checkClickIdle(pressed, diff); break;
      case StateSingleClick: next = _checkSingleClick(pressed, diff); break;
      case StateDoubleClickDebounce: next = _checkDoubleClickDebounce(pressed, diff); break;
      case StateDoubleClick: next = _checkDoubleClick(pressed, diff); break;
      case StateLongClick: next = _checkLongClick(pressed, diff); break;
      case StateOtherUp: next = _checkOtherUp(pressed, diff); break;
    }

    if (next != _state) {
      _lastTransition = now;
      _state = next;

      _new = true;
    }
  }

  bool isClick() const {
    return _new && (_state == StatePressed || _state == StateDoubleClick);
  }

  bool isSingleClick() {
    return _new && _state == StateSingleClick;
  }

  bool isDoubleClick() {
    return _new && _state == StateDoubleClick;
  }

  bool isLongClick() {
    return _new && _state == StateLongClick;
  }

  bool isReleased() {
    return _new && (_state == StateClickUp || _state == StateOtherUp);
  }

  int SINGLECLICK_DELAY = 200;  // ms

private:
  static const int DEBOUNCE_DELAY = 20;  // ms
  // static const int SINGLECLICK_DELAY = 250;  // ms
  static const int LONGCLICK_DELAY = 500;  // ms
  enum State {
    StateIdle,
    StateDebounce,
    StatePressed,
    StateClickUp,
    StateClickIdle,
    StateSingleClick,
    StateDoubleClickDebounce,
    StateDoubleClick,
    StateLongClick,
    StateOtherUp,
  };

  unsigned int _lastTransition;
  State _state;
  bool _new;

  State _checkIdle(bool pressed, int diff) {
    (void)diff;
    return pressed ? StateDebounce : StateIdle;
  }

  State _checkDebounce(bool pressed, int diff) {
    if (!pressed) {
      return StateIdle;
    }
    if (diff >= DEBOUNCE_DELAY) {
      return StatePressed;
    }
    return StateDebounce;
  }

  State _checkPressed(bool pressed, int diff) {
    if (!pressed) {
      return StateClickUp;
    }
    if (diff >= LONGCLICK_DELAY) {
      return StateLongClick;
    }
    return StatePressed;
  }

  State _checkClickUp(bool pressed, int diff) {
    (void)pressed;
    (void)diff;
    return StateClickIdle;
  }

  State _checkClickIdle(bool pressed, int diff) {
    if (pressed) {
      return StateDoubleClickDebounce;
    }
    if (diff >= SINGLECLICK_DELAY) {
      return StateSingleClick;
    }
    return StateClickIdle;
  }

  State _checkSingleClick(bool pressed, int diff) {
    (void)pressed;
    (void)diff;
    return StateIdle;
  }

  State _checkDoubleClickDebounce(bool pressed, int diff) {
    if (!pressed) {
      return StateClickIdle;
    }
    if (diff >= DEBOUNCE_DELAY) {
      return StateDoubleClick;
    }
    return StateDoubleClickDebounce;
  }

  State _checkDoubleClick(bool pressed, int diff) {
    (void)diff;
    if (!pressed) {
      return StateOtherUp;
    }
    return StateDoubleClick;
  }

  State _checkLongClick(bool pressed, int diff) {
    (void)diff;
    if (!pressed) {
      return StateOtherUp;
    }
    return StateLongClick;
  }

  State _checkOtherUp(bool pressed, int diff) {
    (void)pressed;
    (void)diff;
    return StateIdle;
  }
};

class PinButton : public MultiButton {
public:
  PinButton(int pin)
    : MultiButton(), _pin(pin) {
#ifdef ARDUINO_ARCH_STM32
    pinMode(pin, (WiringPinMode)INPUT_PULLUP);
#else
    pinMode(pin, INPUT_PULLUP);
#endif
  }

  PinButton(int pin, int pinType)
    : MultiButton(), _pin(pin) {
#ifdef ARDUINO_ARCH_STM32
    pinMode(pin, (WiringPinMode)pinType);
#else
    pinMode(pin, pinType);
#endif
    if (INPUT == pinType) {
      _pinActiveLevel = HIGH;
    }
  }

  PinButton(int pin, int pinType, int actLvl, long singleDelay = 200)
    : MultiButton(), _pin(pin) {
#ifdef ARDUINO_ARCH_STM32
    pinMode(pin, (WiringPinMode)pinType);
#else
    pinMode(pin, pinType);
#endif
    _pinActiveLevel = actLvl;

    MultiButton::SINGLECLICK_DELAY = singleDelay;
  }

  void update() {
    MultiButton::update(digitalRead(_pin) == _pinActiveLevel);
  }

private:
  int _pin;
  int _pinActiveLevel = LOW;
};
