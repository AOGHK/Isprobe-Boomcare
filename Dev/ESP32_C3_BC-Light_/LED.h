#ifndef _LED_h
#define _LED_h

#include "arduino.h"
#include "Adafruit_NeoPixel.h"

#include "SysEnv.h"

enum {
  LED_POWER_CTRL = 1,
  LED_THEME_CTRL,
  LED_BRIGHTNESS_CTRL,
};

enum {
  LED_STA_CHARGE = 1,
  LED_STA_WIFI_CONN,
  LED_STA_WIFI_DISCONN,
};

struct led_evt_t {
  uint8_t _ctrl;
  uint8_t _themeColors[3];
  uint8_t _brightness;
};

class LEDClass {
public:
  LEDClass();
  void begin();

private:

};

extern LEDClass LED;

#endif