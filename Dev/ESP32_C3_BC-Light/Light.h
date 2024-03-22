#ifndef _LIGHTING_h
#define _LIGHTING_h

#include "arduino.h"

#include "SysConf.h"
#include "Rom.h"
#include "LED.h"
#include "Wi_Fi.h"

class LightClass {
public:
  LightClass();
  bool isRighting();
  void setActivate(bool _enable);
  bool getActivate();

  void on();
  void on(bool _isThermoCtrl);
  void off();

  void powerSwitch();
  void powerSwitch(bool _enable);

  void changeTheme();
  void changeBrightness(bool _isDim);

  void thermoConnect(bool _isConn);
  void thermoMeasure(uint16_t _thermo);

  void startUserTimer(uint16_t _sec);
  void timer();
  
private:
  bool isActivate = false;
  bool isThermoCtrl = false;

  unsigned long thermoLightTime = 0;
  void thermoLightTimer();

  unsigned long userLightTime = 0;
  unsigned long userLightTimeout = 0;
  void userLightTimer();
};

extern LightClass Light;

#endif