#ifndef SYS_ENV_H_
#define SYS_ENV_H_

#include "EEPROM.h"

#define DEBUG_LOG 1

#define TC1_PIN 20
#define TC2_PIN 21
#define PW_LED_PIN 3
#define RED_LED_PIN 1
#define GREEN_LED_PIN 2
#define BLUE_LED_PIN 0
#define RGB_LED_PIN 6
#define LED_LDO_PIN 8
#define SDA_PIN 4
#define SCL_PIN 5
#define BTN_PIN 7

// # BLE
#define BOOMCARE_SCAN_TIME 5
#define BLEC_SCAN_DELAY 100
#define BLES_AD_DELAY 100
#define BLEC_AD_DELAY 500

// # LED
#define RGB_LED_FREQ 5000
#define RGB_LED_BIT 8
#define THEME_SIZE 5
#define LED_MIN_BRIGHTNESS 50
#define LED_MAX_BRIGHTNESS 250
#define THERMO_LIGHT_TIMEOUT 10000

// # Storage
#define EEPROM_SIZE 64

// # Button
#define BTN_LONG_PRESSURE_SIZE 15

enum {
  LIGHT_CTRL_SWITCH = 1,
  LIGHT_CTRL_APP,
  LIGHT_CTRL_DEVICE,
};

enum {
  LED_STA_PW_OFF = 1,
  LED_STA_WIFI_CONNECTED,
  LED_STA_WIFI_ERROR,
};

enum {
  LED_POWER_ON = 1,
  LED_POWER_OFF,
  LED_CHANGE_THEME,
  LED_BRIGHTNESS_CTRL,
  LED_TEMPERATURE_COLOR,
};

enum {
  BLE_BOOMCARE_CONNECT = 1,
  BLE_BOOMCARE_DISCONNECT,
  BLE_MEASURE_TEMPERATURE,
  BLE_RECV_MESSAGE,
};

typedef struct led_ctrl_data {
  uint8_t typeNum;
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  uint8_t brightness;
  bool isDimBrightness;
} led_ctrl_t;

typedef struct ble_evt_data {
  uint8_t typeNum;
  String recvStr;
} ble_evt_t;

extern uint8_t themeColors[THEME_SIZE + 1][3];
extern uint8_t themeNum;
extern uint8_t ledBrightness;
extern String wifiSSID;
extern String wifiPwd;

extern xQueueHandle led_queue;
extern xQueueHandle ble_queue;

class SysConf {
public:
  SysConf();
  void begin();

  void transferLEDEvent(uint8_t evtNum, bool isDimCtrl = false, String thermoStr = "");
private:
  void initRom();
  void readRom();
};

#endif
