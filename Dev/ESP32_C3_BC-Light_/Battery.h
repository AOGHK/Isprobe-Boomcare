#ifndef _BATTERY_h
#define _BATTERY_h

#include "arduino.h"
#include "Wire.h"
#include "Adafruit_MAX1704X.h"

#include "SysEnv.h"

class Battery {
public:
  Battery();
  void begin();
  uint8_t measure();
private:
  Adafruit_MAX17048 maxlipo;
  bool isBatEnable = false;
};

extern Battery Bat;

#endif