#ifndef _LED_h
#define _LED_h

#include "arduino.h"
#include "Adafruit_NeoPixel.h"

#include "SysConf.h"
#include "Rom.h"

enum {
  LED_POWER_CTRL = 1,
  LED_BRIGHTNESS_CTRL,
  LED_GET_BACK,
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

  void setDot(uint32_t _color);

  void lightOn();
  void lightOff();
  void getBack();

  uint8_t getThemeNumber();
  void nextThemeNumber();
  void setThemeNumber(uint8_t _num);
  void setThemeColor(String data);
  String getThemeColor(uint8_t _num);

  uint8_t getBrightness();
  void reducesBrightness();
  void increasesBrightness();
  void setBrightness(uint8_t _brightness, bool _fixed);

  void setThermoColor(uint16_t _thermo);
  void setRGBColor(uint8_t _red, uint8_t _green, uint8_t _blue);

private:
  uint8_t brightness;
  uint8_t themeNum = 0;
  uint8_t themeColors[LED_THEME_SIZE + 1][3];

  uint32_t dotColor;
};

extern LedClass Led;
extern xQueueHandle ledStaQueue;

#endif