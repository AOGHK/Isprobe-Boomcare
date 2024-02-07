#ifndef _LIGHTING_h
#define _LIGHTING_h

#include "arduino.h"

#include "SysConf.h"
#include "Rom.h"
#include "LED.h"

class LightClass {
public:
  LightClass();
  bool isRighting();

  void on();
  void on(bool _isThermoCtrl);
  void off();

  void powerSwitch();

  void changeTheme();
  void changeBrightness(bool _isDim);


  void thermoConnect(bool _isConn);
  void thermoMeasure(uint16_t _thermo);

  void timer();
private:
  bool isActivate = false;
  bool isThermoCtrl = false;

  unsigned long thermoLightTime = 0;
  void thermoLightTimer();
};

extern LightClass Light;

#endif