#ifndef _ROM_h
#define _ROM_h

#include "arduino.h"
#include "EEPROM.h"

#include "SysConf.h"

class RomClass {
public:
  RomClass();
  void begin();
  void clear();

  void getLedAttribute(uint8_t* _brightness, uint8_t* _themeNum, uint8_t (*_color)[3]);
  void setBrightness(uint8_t _brightness);
  void setThemeNumber(uint8_t _themeNum);
  void setThemeColor(uint8_t _themeNum, uint8_t _red, uint8_t _green, uint8_t _blue);

private:
  void init();
};

extern RomClass Rom;

#endif