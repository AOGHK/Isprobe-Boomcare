#ifndef _LED_h
#define _LED_h

#include "arduino.h"
#include "Adafruit_NeoPixel.h"

#include "SysEnv.h"
#include "ROM.h"

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
  uint8_t brightness;
  uint8_t themeNum = 0;
  uint8_t themeColors[THEME_SIZE + 1][3];
};

extern LEDClass LED;

#endif