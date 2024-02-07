#ifndef _LED_h
#define _LED_h

#include "arduino.h"
#include "Adafruit_NeoPixel.h"

#include "SysConf.h"
#include "Rom.h"

enum {
  LED_POWER_CTRL = 1,
  LED_BRIGHTNESS_CTRL,
};

struct led_ctrl_t {
  uint8_t type;
  uint8_t colors[3];
  uint8_t brightness;
};

class LedClass {
public:
  LedClass();
  void begin();
  void clear();

  void lightOn();
  void lightOff();

  uint8_t getThemeNumber();
  void nextThemeNumber();
  void setThemeNumber(uint8_t _num);

  uint8_t getBrightness();
  void reducesBrightness();
  void increasesBrightness();

  void setThermoColor(uint16_t _thermo);

private:
  uint8_t brightness;
  uint8_t themeNum = 0;
  uint8_t themeColors[LED_THEME_SIZE + 1][3];
};

extern LedClass Led;

#endif