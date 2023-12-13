#ifndef _MYWIFI_h
#define _MYWIFI_h

#include "SysEnv.h"

#include "arduino.h"
#include "WiFi.h"
#include "HTTPClient.h"
#include "EEPROM.h"
#include "time.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

enum {
  WIFI_STA_REQ_CONNECT = 1,
  WIFI_STA_CONNECTING,
  WIFI_STA_CONNECTED,
};

typedef struct tmp_param_data {
  uint8_t tmp[2];
  uint8_t time[6];
} tmp_param_t;

class MyWiFi {
public:
  MyWiFi();
  void begin();
  void renewalConnect(String _SSID, String _Pwd);
  void uploadThermoValue(uint16_t _value);
  void updateRom();

  void setCallback(void (*stateCallback)(bool, bool));
private:
  void readRom();
};

#endif