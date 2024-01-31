#ifndef _BATTERY_h
#define _BATTERY_h

#include "arduino.h"
#include "Wire.h"
#include "Adafruit_MAX1704X.h"

#include "SysEnv.h"
#include "LED.h"

class Battery {
public:
  Battery();
  void begin();
  void scan();

  uint8_t getLevel();
private:
  Adafruit_MAX17048 maxlipo;
  bool isEnable = false;
  uint8_t level;
  unsigned long measureTime = 0;
};

extern Battery Bat;

#endif