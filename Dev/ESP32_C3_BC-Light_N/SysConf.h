#ifndef _SYS_CONF_H_
#define _SYS_CONF_H_

#include <stdint.h>

#define DEBUG_LOG 1

// ROM
#define ROM_SIZE 256
#define ADDR_WIFI_SSID_LEN 99

// GPIO
#define PW_STA_PIN 7
#define PW_BTN_PIN 10
#define PW_CTRL_PIN 8
#define BOT_TC_PIN 20  // TC1
#define TOP_TC_PIN 21  // TC2
#define SDA_PIN 4
#define SCL_PIN 5

// BLE
#define BLEC_SCAN_SEC 5
#define BLEC_SCAN_DELAY 100
#define BLES_AD_DELAY 100

// WiFi
#define WIFI_STA_SYNC_TIMER 1000
#define WIFI_PING_TIMER (1000 * 60)

// LED
#define PW_LED_PIN 3
#define RED_LED_PIN 1
#define GREEN_LED_PIN 2
#define BLUE_LED_PIN 0
#define STA_LED_PIN 6
#define RGB_LED_FREQ 5000  // (1000000 * 20)
#define RGB_LED_BIT 8
#define LED_THEME_SIZE 5
#define LED_MIN_BRIGHTNESS 100
#define LED_MAX_BRIGHTNESS 255
#define THERMO_LIGHT_TIMEOUT 3000
#define LED_CTRL_STEP_SIZE 1
#define DOT_RED_COLOR 4194304
#define DOT_GREEN_COLOR 16384
#define DOT_BLUE_COLOR 64

// BATTERY
#define SCAN_BATTERY_TIMER (1000 * 20)
#define LOW_BATTERY_LIMIT 10
#define LOW_BAT_BLINK_SIZE 4
#define LOW_BAT_BLINK_DELAY 200

struct thermo_data_t {
  uint8_t val[2];
  uint8_t time[6];
};

#endif /*_SYS_CONF_H_*/