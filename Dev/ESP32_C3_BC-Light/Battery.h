#ifndef _BATTERY_h
#define _BATTERY_h

#include "arduino.h"
#include "Wire.h"
#include "Adafruit_MAX1704X.h"
#include "GPIO_Pin.h"
#include "Light.h"

#define SCAN_BATTERY_TIMER (1000 * 20)
#define LOW_BATTERY_LIMIT 10

class Battery {
public:
  Battery();
  void init();

  void scan();
  void resetTime();

  uint8_t getLevel();

private:
  Adafruit_MAX17048 maxlipo;
  uint8_t lvl;
  unsigned long scanTime;
  void checkLowLevel();
};

extern Battery Bat;

#endif