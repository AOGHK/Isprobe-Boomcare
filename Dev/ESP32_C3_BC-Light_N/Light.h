#ifndef _LIGHTING_h
#define _LIGHTING_h

#include "arduino.h"
#include "Rom.h"
#include "LED.h"
#include "GPIO_Pin.h"

#define THERMO_LIGHT_TIMEOUT 3000

class LightClass {
public:
  LightClass();

  bool isActivated();

  void on();
  void on(bool _isThermoCtrl);
  void off();

  void powerSwitch();
  void powerSwitch(bool _enable);

  void changeTheme();
  void changeThemeColor(uint8_t _num, uint8_t _red, uint8_t _green, uint8_t _blue, uint8_t _isFixed);

  void changeBrightness(bool _isDim);
  void setBrightness(uint8_t _brightness, uint8_t _isFixed);

  void setUserTimer(uint16_t _sec);
  void run();

  void lowBattery();

  void thermoConnection(bool _isConn);
  void thermoMeasurement(uint16_t _thermo);

private:
  bool isActivate = false;
  bool isThermoCtrl = false;

  unsigned long thermoLightTime = 0;
  void thermoTimer();

  unsigned long userLightTime = 0;
  unsigned long userLightTimeout = 0;
  void userTimer();
};

extern LightClass Light;

#endif