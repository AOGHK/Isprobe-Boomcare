#ifndef _LED_h
#define _LED_h

#include "arduino.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "EEPROM.h"
#include "Adafruit_NeoPixel.h"

#define PW_LED_PIN 3
#define RED_LED_PIN 1
#define GREEN_LED_PIN 2
#define BLUE_LED_PIN 0
#define STA_LED_PIN 6

#define RGB_LED_FREQ 5000  // (1000000 * 20)
#define RGB_LED_BIT 8
#define THEME_SIZE 5
#define LED_MIN_BRIGHTNESS 100
#define LED_MAX_BRIGHTNESS 255
#define THERMO_LIGHT_TIMEOUT 3000
#define STA_LED_BRIGHTNESS 64
#define CTRL_STEP_SIZE 5
#define ALIVE_BLINK_INTERVAL 1000

#define EEPROM_SIZE 256

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
  void setTemperature(uint16_t value);
  void setThemeColor(String data);
  void setBrightness(uint8_t data);
  String getThemeData(char type);

  void ctrlPower(bool isOn);
  void changeTheme();
  void changeBrightness(bool isDim);

  void saveBrightness();
  void saveThemeNumber();
  void saveThemeColor();

  void aliveBlink();
  void clear();

  void initAction();
  void lowBattery(uint8_t _blinkCnt, uint16_t delay_ms);
private:
  uint8_t brightness;
  uint8_t themeNum = 0;
  uint8_t themeColors[THEME_SIZE + 1][3];
  
  uint32_t staColor = 0;

  uint8_t aliveCnt;
  unsigned long aliveTime;

  void initRom();
  void bindingData();
};

#endif