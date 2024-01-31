#include "ROM.h"

ROMClass ROM;

ROMClass::ROMClass() {
}

void ROMClass::begin() {
  EEPROM.begin(ROM_SIZE);
  if (EEPROM.read(0) != 1) {
    EEPROM.write(0, 1);
    init();
  }
}

void ROMClass::init() {
  EEPROM.write(0, 1);
  EEPROM.write(1, 150);  // Power Led Brightness
  EEPROM.write(2, 0);    // Theme Num = Default 0 (Only Power LED)
  EEPROM.write(3, 255);  // RGB Theme 1
  EEPROM.write(4, 0);
  EEPROM.write(5, 0);
  EEPROM.write(6, 0);  // RGB Theme 2
  EEPROM.write(7, 255);
  EEPROM.write(8, 0);
  EEPROM.write(9, 0);  // RGB Theme 3
  EEPROM.write(10, 0);
  EEPROM.write(11, 255);
  EEPROM.write(12, 0);  // RGB Theme 4
  EEPROM.write(13, 255);
  EEPROM.write(14, 255);
  EEPROM.write(15, 255);  // RGB Theme 5
  EEPROM.write(16, 0);
  EEPROM.write(17, 255);
  for (uint16_t i = 18; i < ROM_SIZE; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
}

void ROMClass::clear() {
  init();
}

void ROMClass::setWiFi(String _ssid, String _pwd) {
  uint8_t ssidLen = _ssid.length();
  uint8_t pwdLen = _pwd.length();
  EEPROM.write(ADDR_WIFI_SSID_LEN, ssidLen);
  EEPROM.write(ADDR_WIFI_SSID_LEN + 1, pwdLen);

  uint8_t addr = ADDR_WIFI_SSID_LEN + 2;
  for (uint8_t idx = 0; idx < ssidLen; idx++) {
    EEPROM.write(addr + idx, _ssid[idx]);
  }
  addr = addr + ssidLen;
  for (uint8_t idx = 0; idx < pwdLen; idx++) {
    EEPROM.write(addr + idx, _pwd[idx]);
  }
  bool res = EEPROM.commit();

#if DEBUG_LOG
  Serial.printf("WiFi Saved : %d\n", res);
#endif
}

void ROMClass::getWiFi(String* _ssid, String* _pwd) {
  uint8_t ssidLen = EEPROM.read(ADDR_WIFI_SSID_LEN);
  uint8_t pwdLen = EEPROM.read(ADDR_WIFI_SSID_LEN + 1);
  uint8_t addr = ADDR_WIFI_SSID_LEN + 2;
  String ssid = "";
  for (uint8_t idx = 0; idx < ssidLen; idx++) {
    ssid += (char)EEPROM.read(addr + idx);
  }
  addr = addr + ssidLen;
  String pwd = "";
  for (uint8_t idx = 0; idx < pwdLen; idx++) {
    pwd += (char)EEPROM.read(addr + idx);
  }
  *_ssid = ssid;
  *_pwd = pwd;
}


void ROMClass::getLedAttribute(uint8_t* _brightness, uint8_t* _themeNum, uint8_t (*_color)[3]) {
  *_brightness = EEPROM.read(1);
  *_themeNum = EEPROM.read(2);
  _color[0][0] = 0;
  _color[0][1] = 0;
  _color[0][2] = 0;
  for (uint8_t i = 3; i < 18; i += 3) {
    uint8_t pos = i / 3;
    _color[pos][0] = EEPROM.read(i);
    _color[pos][1] = EEPROM.read(i + 1);
    _color[pos][2] = EEPROM.read(i + 2);
  }
}


void ROMClass::setBrightness(uint8_t _brightness) {
  EEPROM.write(1, _brightness);
  EEPROM.commit();
}

void ROMClass::setThemeNumber(uint8_t _themeNum) {
  EEPROM.write(2, _themeNum);
  EEPROM.commit();
}

void ROMClass::setThemeColor(uint8_t _themeNum, uint8_t _red, uint8_t _green, uint8_t _blue) {
  EEPROM.write(2, _themeNum);
  uint8_t startAddr = _themeNum * 3;
  EEPROM.write(startAddr, _red);
  EEPROM.write(startAddr + 1, _green);
  EEPROM.write(startAddr + 2, _blue);
  EEPROM.commit();
}
