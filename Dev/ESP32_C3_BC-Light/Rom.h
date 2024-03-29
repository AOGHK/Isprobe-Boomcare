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

  void setWiFi(String _ssid, String _pwd);
  void getWiFi(String* _ssid, String* _pwd);

  void addBackupThermo(thermo_data_t* _data, uint8_t _idx, uint8_t _totalSize);
  void getBackupThermos(thermo_data_t* _data, uint8_t* _size);
  void clearBakcupThermos(uint8_t _size);
  
private:
  void init();
};

extern RomClass Rom;

#endif