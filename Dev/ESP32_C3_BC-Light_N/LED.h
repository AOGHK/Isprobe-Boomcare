#ifndef _LED_h
#define _LED_h

#include "arduino.h"
#include "Adafruit_NeoPixel.h"
#include "GPIO_Pin.h"
#include "Rom.h"

#include "Proc.h"

#define RGB_LED_FREQ 5000
#define RGB_LED_BIT 8

#define LED_MIN_BRIGHTNESS 100
#define LED_MAX_BRIGHTNESS 255

#define DOT_RED_COLOR 4194304
#define DOT_GREEN_COLOR 16384
#define DOT_BLUE_COLOR 64

#define LED_TAG "LED"

enum {
  LED_COLOR_CTRL = 1,
  LED_BRIGHTNESS_CTRL,
  LED_THERMO_TIMEOUT,
};

typedef struct {
  uint8_t type;
  led_theme_t colors;
  uint8_t brightness;
} led_ctrl_t;

class LedClass {
public:
  LedClass();
  void begin();
  void clear();

  void lightOn();
  void lightOn(uint8_t _type);
  void lightOff();

  uint8_t getThemeNumber();
  void changeThemeNumber();
  void setThemeNumber(uint8_t _num);

  void setThemeColor(uint8_t _num, uint8_t _red, uint8_t _green, uint8_t _blue, bool _isFixed);
  led_theme_t* getThemeColor();

  uint8_t getBrightness();
  void setBrightness(uint8_t _brightness, bool _isFixed);
  void reducesBrightness();
  void increasesBrightness();

  void setThermoColor(uint16_t _thermo);

  void setDot(uint32_t _color);
  void setLedColor(uint8_t _red, uint8_t _green, uint8_t _blue);

  void infoLog();

private:
  uint8_t brightness;
  uint8_t themeNum = 0;
  led_theme_t themeColors[3] = { 0 };
};

extern LedClass Led;

#endif