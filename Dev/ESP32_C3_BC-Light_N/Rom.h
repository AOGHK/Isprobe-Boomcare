#ifndef _ROM_h
#define _ROM_h

#include "arduino.h"
#include <Preferences.h>

#define KEY_INIT_STA "INIT"
#define KEY_LED_BRIGHTNESS "BRIGHTNESS"
#define KEY_LED_THEME_NUM "THEME_NUM"
#define KEY_LED_THEME_COLOR "THEME_COLOR"
#define KEY_WIFI_SSID "WIFI_SSID"
#define KEY_WIFI_PWD "WIFI_PWD"

#define KEY_TEMP_DATETIME "TEMP_DATETIME"
#define KEY_TEMP_VALUE "TEMP_VALUE"

struct led_theme_t {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};

struct temp_date_t {
  uint8_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t min;
  uint8_t sec;
};

struct temp_value_t {
  uint8_t integer;
  uint8_t point;
};

class RomClass {
public:
  RomClass();

  void setBrightness(uint8_t _brightness);
  void setThemeNumber(uint8_t _themeNum);
  void setThemeColors(led_theme_t* _colors);

  void getLedAttribute(uint8_t* _brightness, uint8_t* _themeNum, led_theme_t* _colors);

  void getWiFi(String* _ssid, String* _pwd);
  void setWiFi(String _ssid, String _pwd);

  size_t getTemperatureSize();

  void clearTemperature(const char* _name);
  void clearTemperature();

  void addTemperatureDatetime(temp_date_t* _datetime);
  void addTemperatureValue(temp_value_t* _value);
  void addTemperature(temp_date_t* _datetime, temp_value_t* _value);
  
  void getTemperatureDatetime(temp_date_t* _datetime);
  void getTemperatureValue(temp_value_t* _value);
  void getTemperature(temp_date_t* _datetime, temp_value_t* _value);

private:
  uint8_t getBrightness();
  void getThemeColors(led_theme_t* _colors);
  uint8_t getThemeNumber();

  String getWiFiName();
  void setWiFiName(String _ssid);

  String getWiFiPassword();
  void setWiFiPassword(String _pwd);
};

extern RomClass Rom;

#endif