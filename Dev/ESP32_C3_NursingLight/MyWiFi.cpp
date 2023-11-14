#include "MyWiFi.h"

#define WIFI_STATE_NO_DATA 0
#define WIFI_STAET_CONNECTING 1
#define WIFI_STAET_CONNECTED 2
#define WIFI_STAET_DISONNECT 3

uint8_t wStateNum;

unsigned long wifi_scan_time = 0;

MyWiFi::MyWiFi() {
}

void MyWiFi::sync() {
  if (WiFi.isConnected()) {
    WiFi.disconnect();
  }

  if (wifiSSID.length() == 0) {
    wStateNum = WIFI_STATE_NO_DATA;
  } else {
    WiFi.begin(wifiSSID.c_str(), wifiPwd.c_str());
    wStateNum = WIFI_STAET_CONNECTING;
  }
}

void MyWiFi::checkConnected() {
}