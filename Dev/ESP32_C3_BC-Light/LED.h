#ifndef _LED_h
#define _LED_h

#include "SysEnv.h"

#include "arduino.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "EEPROM.h"
#include "Adafruit_NeoPixel.h"

enum {
  LED_POWER_CTRL = 1,
  LED_THEME_CTRL,
  LED_BRIGHTNESS_CTRL,
};

enum {
  LED_STA_CHARGE = 1,
  LED_STA_WIFI_CONN,
  LED_STA_WIFI_DISCONN,
};

typedef struct led_evt_data {
  uint8_t _ctrl;
  uint8_t _themeColors[3];
  uint8_t _brightness;
} led_evt_t;

class LED {
public:
  LED();
  void begin();

  void setState(uint8_t _sta);
  void setThermoColor(uint16_t value);
  void setThemeColor(String data);
  void setBrightness(uint8_t data);

  String getPreferences(char type);

  void ctrlPower(bool isOn);
  void changeTheme();
  void changeBrightness(bool isDim);

  void saveBrightness();

  void clear();
  void startAct();
  void lowBattery(uint8_t _blinkCnt, uint16_t delay_ms);
private:
  uint8_t brightness;
  uint8_t themeNum = 0;
  uint8_t themeColors[THEME_SIZE + 1][3];

  void bindingData();
  void saveThemeNumber();
  void saveThemeColor();
};

#endif