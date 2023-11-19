#ifndef _MYWIFI_h
#define _MYWIFI_h

#include "arduino.h"
#include "WiFi.h"
#include "HTTPClient.h"
#include "EEPROM.h"
#include "time.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#define WIFI_STA_SCAN_TIMER 1000
#define WIFI_CONNECT_ERR_CNT 5

enum {
  WIFI_STA_REQ_CONNECT = 1,
  WIFI_STA_CONNECTING,
  WIFI_STA_CONNECTED,
};

class MyWiFi {
public:
  MyWiFi();
  void begin();
  void updateRom();
  void renewalData(String _SSID, String _Pwd);
  void setConnectCallback(void (*evtCallback)(bool, bool));
  void scanState();

  void syncNTPTime();
  String getCurrentTime();
private:
  void readRom();
  void scanConnected();
};

#endif
