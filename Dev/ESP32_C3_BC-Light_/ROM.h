#ifndef _ROM_h
#define _ROM_h

#include "arduino.h"
#include "EEPROM.h"

#include "SysEnv.h"

#define ROM_SIZE 256
#define ADDR_WIFI_SSID_LEN 99

class ROMClass {
public:
  ROMClass();
  void begin();
  void clear();

  void setWiFi(String _ssid, String _pwd);
  void getWiFi(String* _ssid, String* _pwd);

  void getLedAttribute(uint8_t* _brightness, uint8_t* _themeNum, uint8_t (*_color)[3]);

private:
  void init();
};

extern ROMClass ROM;

#endif