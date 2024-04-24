#include "Rom.h"

RomClass Rom;

RomClass::RomClass() {
}


uint8_t RomClass::getBrightness() {
  Preferences prefs;
  prefs.begin(KEY_LED_BRIGHTNESS);
  uint8_t _brightness = prefs.getUChar(KEY_LED_BRIGHTNESS, 150);
  prefs.end();
  return _brightness;
}

void RomClass::setBrightness(uint8_t _brightness) {
  Preferences prefs;
  prefs.begin(KEY_LED_BRIGHTNESS);
  prefs.putUChar(KEY_LED_BRIGHTNESS, _brightness);
  prefs.end();
}

uint8_t RomClass::getThemeNumber() {
  Preferences prefs;
  prefs.begin(KEY_LED_THEME_NUM);
  uint8_t _themeNum = prefs.getUChar(KEY_LED_THEME_NUM, 0);
  prefs.end();
  return _themeNum;
}

void RomClass::setThemeNumber(uint8_t _themeNum) {
  Preferences prefs;
  prefs.begin(KEY_LED_THEME_NUM);
  prefs.putUChar(KEY_LED_THEME_NUM, _themeNum);
  prefs.end();
}

void RomClass::getThemeColors(led_theme_t *_colors) {
  Preferences prefs;
  prefs.begin(KEY_LED_THEME_COLOR);
  size_t _len = prefs.getBytesLength(KEY_LED_THEME_COLOR);
  led_theme_t *colors;
  if (_len == 0) {
    uint8_t defaultColors[] = { 255, 0, 0, 0, 255, 0, 0, 0, 255 };
    prefs.putBytes(KEY_LED_THEME_COLOR, defaultColors, 9);
    colors = (led_theme_t *)defaultColors;
  } else {
    uint8_t colorArry[_len];
    prefs.getBytes(KEY_LED_THEME_COLOR, colorArry, _len);
    colors = (led_theme_t *)colorArry;
  }
  for (int i = 0; i < 3; i++) {
    _colors[i].red = colors[i].red;
    _colors[i].green = colors[i].green;
    _colors[i].blue = colors[i].blue;
  }
  prefs.end();
}

void RomClass::setThemeColors(led_theme_t *_colors) {
  Preferences prefs;
  prefs.begin(KEY_LED_THEME_COLOR);
  prefs.putBytes(KEY_LED_THEME_COLOR, _colors, 3 * sizeof(led_theme_t));
  prefs.end();
}

void RomClass::getLedAttribute(uint8_t *_brightness, uint8_t *_themeNum, led_theme_t *_colors) {
  *_brightness = getBrightness();
  *_themeNum = getThemeNumber();
  getThemeColors(_colors);
}

String RomClass::getWiFiName() {
  Preferences prefs;
  prefs.begin(KEY_WIFI_SSID);
  String _name = prefs.getString(KEY_WIFI_SSID, "");
  prefs.end();
  return _name;
}

void RomClass::setWiFiName(String _ssid) {
  Preferences prefs;
  prefs.begin(KEY_WIFI_SSID);
  prefs.putString(KEY_WIFI_SSID, _ssid);
  prefs.end();
}

String RomClass::getWiFiPassword() {
  Preferences prefs;
  prefs.begin(KEY_WIFI_PWD);
  String _pwd = prefs.getString(KEY_WIFI_PWD, "");
  prefs.end();
  return _pwd;
}

void RomClass::setWiFiPassword(String _pwd) {
  Preferences prefs;
  prefs.begin(KEY_WIFI_PWD);
  prefs.putString(KEY_WIFI_PWD, _pwd);
  prefs.end();
}

void RomClass::getWiFi(String *_ssid, String *_pwd) {
  *_ssid = getWiFiName();
  *_pwd = getWiFiPassword();
}

void RomClass::setWiFi(String _ssid, String _pwd) {
  setWiFiName(_ssid);
  setWiFiPassword(_pwd);
}

size_t RomClass::getTemperatureSize() {
  Preferences prefs;
  prefs.begin(KEY_TEMP_VALUE);
  size_t _len = prefs.getBytesLength(KEY_TEMP_VALUE);
  prefs.end();
  return _len / 2;
}

void RomClass::clearTemperature(const char *_name) {
  Preferences prefs;
  prefs.begin(_name);
  prefs.clear();
  prefs.end();
}

void RomClass::clearTemperature() {
  clearTemperature(KEY_TEMP_DATETIME);
  clearTemperature(KEY_TEMP_VALUE);
}


void RomClass::addTemperatureDatetime(temp_date_t *_datetime) {
  Preferences prefs;
  prefs.begin(KEY_TEMP_DATETIME);
  size_t _len = prefs.getBytesLength(KEY_TEMP_DATETIME);

  if (_len == 0) {
    prefs.putBytes(KEY_TEMP_DATETIME, _datetime, sizeof(temp_date_t));
  } else {
    uint8_t buf[_len];
    prefs.getBytes(KEY_TEMP_DATETIME, buf, _len);
    temp_date_t *_dates = (temp_date_t *)buf;
    if (_len < 60) {
      uint8_t _idx = _len / 6;
      _dates[_idx] = *_datetime;
      prefs.putBytes(KEY_TEMP_DATETIME, _dates, (_idx + 1) * sizeof(temp_date_t));
    } else {
      for (size_t i = 1; i < 10; i++) {
        _dates[i - 1] = _dates[i];
      }
      _dates[9] = *_datetime;
      prefs.putBytes(KEY_TEMP_DATETIME, _dates, 10 * sizeof(temp_date_t));
    }
  }
  prefs.end();
}

void RomClass::addTemperatureValue(temp_value_t *_value) {
  Preferences prefs;
  prefs.begin(KEY_TEMP_VALUE);
  size_t _len = prefs.getBytesLength(KEY_TEMP_VALUE);

  if (_len == 0) {
    prefs.putBytes(KEY_TEMP_VALUE, _value, sizeof(temp_value_t));
  } else {
    uint8_t buf[_len];
    prefs.getBytes(KEY_TEMP_VALUE, buf, _len);
    temp_value_t *_dates = (temp_value_t *)buf;
    if (_len < 20) {
      uint8_t _idx = _len / 2;
      _dates[_idx] = *_value;
      prefs.putBytes(KEY_TEMP_VALUE, _dates, (_idx + 1) * sizeof(temp_value_t));
    } else {
      for (size_t i = 1; i < 10; i++) {
        _dates[i - 1] = _dates[i];
      }
      _dates[9] = *_value;
      prefs.putBytes(KEY_TEMP_VALUE, _dates, 10 * sizeof(temp_value_t));
    }
  }
  prefs.end();
}

void RomClass::addTemperature(temp_date_t *_datetime, temp_value_t *_value) {
  addTemperatureDatetime(_datetime);
  addTemperatureValue(_value);
}

void RomClass::getTemperatureDatetime(temp_date_t *_datetime) {
  Preferences prefs;
  prefs.begin(KEY_TEMP_DATETIME);
  uint8_t _len = prefs.getBytesLength(KEY_TEMP_DATETIME);
  uint8_t _buf[_len];
  prefs.getBytes(KEY_TEMP_DATETIME, _buf, _len);
  temp_date_t *_dt = (temp_date_t *)_buf;
  memcpy(_datetime, _dt, _len);
  prefs.end();
}

void RomClass::getTemperatureValue(temp_value_t *_value) {
  Preferences prefs;
  prefs.begin(KEY_TEMP_VALUE);
  uint8_t _len = prefs.getBytesLength(KEY_TEMP_VALUE);
  uint8_t _buf[_len];
  prefs.getBytes(KEY_TEMP_VALUE, _buf, _len);
  temp_value_t *_val = (temp_value_t *)_buf;
  memcpy(_value, _val, _len);
  prefs.end();
}

void RomClass::getTemperature(temp_date_t *_datetime, temp_value_t *_value) {
  getTemperatureDatetime(_datetime);
  getTemperatureValue(_value);
}
