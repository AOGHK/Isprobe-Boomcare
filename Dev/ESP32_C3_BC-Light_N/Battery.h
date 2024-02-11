#ifndef _BATTERY_h
#define _BATTERY_h

#include "arduino.h"
#include "Wire.h"
#include "Adafruit_MAX1704X.h"

#include "SysConf.h"
#include "LED.h"

class Battery {
public:
  Battery();
  void init();
  void scan();

  uint8_t getLevel();
private:
  Adafruit_MAX17048 maxlipo;

  uint8_t lvl;
  unsigned long scanTime;

  void checkLowLevel();
};

extern Battery Bat;

#endif