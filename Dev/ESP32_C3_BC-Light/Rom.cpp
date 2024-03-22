#include "Rom.h"

RomClass Rom;

RomClass::RomClass() {
}

void RomClass::begin() {
  EEPROM.begin(ROM_SIZE);
  if (EEPROM.read(0) != 1) {
    EEPROM.write(0, 1);
    init();
  }
}

void RomClass::init() {
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

void RomClass::clear() {
  init();
}

void RomClass::getLedAttribute(uint8_t* _brightness, uint8_t* _themeNum, uint8_t (*_color)[3]) {
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

void RomClass::setBrightness(uint8_t _brightness) {
  EEPROM.write(1, _brightness);
  EEPROM.commit();
}

void RomClass::setThemeNumber(uint8_t _themeNum) {
  EEPROM.write(2, _themeNum);
  EEPROM.commit();
}

void RomClass::setThemeColor(uint8_t _themeNum, uint8_t _red, uint8_t _green, uint8_t _blue) {
  EEPROM.write(2, _themeNum);
  uint8_t _addr = _themeNum * 3;
  EEPROM.write(_addr, _red);
  EEPROM.write(_addr + 1, _green);
  EEPROM.write(_addr + 2, _blue);
  EEPROM.commit();
}


void RomClass::setWiFi(String _ssid, String _pwd) {
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
  EEPROM.commit();
}

void RomClass::getWiFi(String* _ssid, String* _pwd) {
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

void RomClass::addBackupThermo(thermo_data_t* _data, uint8_t _idx, uint8_t _totalSize) {
  EEPROM.write(18, _totalSize);
  uint8_t addr = _idx * 8 + 19;
  EEPROM.write(addr, _data->time[0]);
  EEPROM.write(addr + 1, _data->time[1]);
  EEPROM.write(addr + 2, _data->time[2]);
  EEPROM.write(addr + 3, _data->time[3]);
  EEPROM.write(addr + 4, _data->time[4]);
  EEPROM.write(addr + 5, _data->time[5]);
  EEPROM.write(addr + 6, _data->val[0]);
  EEPROM.write(addr + 7, _data->val[1]);
  EEPROM.commit();
}

void RomClass::getBackupThermos(thermo_data_t* _data, uint8_t* _size) {
  uint8_t size = EEPROM.read(18);
  for (uint8_t i = 0; i < size; i++) {
    uint8_t addr = i * 8 + 19;
    _data[i].time[0] = EEPROM.read(addr);
    _data[i].time[1] = EEPROM.read(addr + 1);
    _data[i].time[2] = EEPROM.read(addr + 2);
    _data[i].time[3] = EEPROM.read(addr + 3);
    _data[i].time[4] = EEPROM.read(addr + 4);
    _data[i].time[5] = EEPROM.read(addr + 5);
    _data[i].val[0] = EEPROM.read(addr + 6);
    _data[i].val[1] = EEPROM.read(addr + 7);
  }
  *_size = size;
}

void RomClass::clearBakcupThermos(uint8_t _size) {
  uint8_t endAddr = (_size - 1) * 8 + 26;
  for (uint8_t i = 18; i < endAddr; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
}