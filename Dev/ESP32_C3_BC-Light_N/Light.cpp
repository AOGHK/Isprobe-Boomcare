#include "Light.h"

LightClass Light;

LightClass::LightClass() {
}

bool LightClass::isRighting() {
  return isActivate;
}

void LightClass::setActivate(bool _enable) {
  isActivate = _enable;
}

bool LightClass::getActivate() {
  return isActivate;
}

void LightClass::on() {
  // if (isActivate) {
  //   return;
  // }
  isActivate = true;
  Led.lightOn();
}

void LightClass::on(bool _isThermoCtrl) {
  if (isActivate) {
    return;
  }
  isThermoCtrl = _isThermoCtrl;
  isActivate = true;
  Led.lightOn();
}

void LightClass::off() {
  // if (!isActivate) {
  //   return;
  // }
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
  Led.nextThemeNumber();
}

void LightClass::changeBrightness(bool _isDim) {
  if (_isDim) {
    Led.reducesBrightness();
  } else {
    Led.increasesBrightness();
  }
}

void LightClass::thermoConnect(bool _isConn) {
  if (_isConn) {
    on(true);
  } else if (isThermoCtrl) {
    off();
  }
}

void LightClass::thermoMeasure(uint16_t _thermo) {
  if (!isActivate) {
    return;
  }
  Led.setThermoColor(_thermo);
  thermoLightTime = millis();
}

void LightClass::thermoLightTimer() {
  if (thermoLightTime == 0) {
    return;
  }

  if (millis() - thermoLightTime > THERMO_LIGHT_TIMEOUT) {
    Led.lightOn();
    thermoLightTime = 0;
  }
}

void LightClass::startUserTimer(uint16_t _sec) {
  userLightTimeout = _sec * 1000;
  userLightTime = millis();
  on();
}

void LightClass::userLightTimer() {
  if (userLightTime == 0) {
    return;
  }

  if (millis() - userLightTime > userLightTimeout) {
    off();
  }
}

void LightClass::timer() {
  thermoLightTimer();
  userLightTimer();
}
