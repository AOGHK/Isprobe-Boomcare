#include "Light.h"

LightClass Light;

LightClass::LightClass() {
}

bool LightClass::isActivated() {
  return isActivate;
}

void LightClass::on() {
  isActivate = true;
  Led.lightOn();
}

void LightClass::on(bool _isThermoCtrl) {
  if (isActivate) {
    return;
  }
  isThermoCtrl = _isThermoCtrl;
  on();
}

void LightClass::off() {
  isThermoCtrl = false;
  isActivate = false;
  thermoLightTime = 0;
  userLightTime = 0;
  Led.lightOff();
}

void LightClass::powerSwitch() {
  if (isActivate) {
    off();
  } else {
    on();
  }
}

void LightClass::powerSwitch(bool _enable) {
  if (!_enable) {
    off();
  } else if (!isActivate) {
    on();
  }
}

void LightClass::changeTheme() {
  if (!isActivate) {
    return;
  }
  Led.changeThemeNumber();
}

void LightClass::changeThemeColor(uint8_t _num, uint8_t _red, uint8_t _green, uint8_t _blue, uint8_t _isFixed) {
  if (_num == 0) {
    return;
  }
  isActivate = true;
  Led.setThemeColor(_num, _red, _green, _blue, _isFixed);
}

void LightClass::changeBrightness(bool _isDim) {
  if (!isActivate) {
    return;
  }
  if (_isDim) {
    Led.reducesBrightness();
  } else {
    Led.increasesBrightness();
  }
}

void LightClass::setBrightness(uint8_t _brightness, uint8_t _isFixed) {
  isActivate = true;
  Led.setBrightness(_brightness, _isFixed);
}

void LightClass::thermoTimer() {
  if (thermoLightTime == 0) {
    return;
  }
  if (millis() - thermoLightTime > THERMO_LIGHT_TIMEOUT) {
    Led.lightOn(LED_THERMO_TIMEOUT);
    thermoLightTime = 0;
  }
}

void LightClass::userTimer() {
  if (userLightTime == 0) {
    return;
  }
  if (millis() - userLightTime > userLightTimeout) {
    off();
  }
}

void LightClass::setUserTimer(uint16_t _sec) {
  userLightTimeout = _sec * 1000;
  userLightTime = millis();
  on();
}

void LightClass::run() {
  thermoTimer();
  userTimer();
}

void LightClass::lowBattery() {
  for (uint8_t cnt = 0; cnt < 6; cnt++) {
    if (cnt % 2 == 0) {
      Led.setLedColor(255, 255, 255);
    } else {
      Led.setLedColor(0, 0, 0);
    }
    delay(200);
  }
}

void LightClass::thermoConnection(bool _isConn) {
  if (_isConn) {
    on(true);
  } else if (isThermoCtrl) {
    off();
  }
}

void LightClass::thermoMeasurement(uint16_t _thermo) {
  if (!isActivate) {
    return;
  }
  Led.setThermoColor(_thermo);
  thermoLightTime = millis();
}
