#include "MyWiFi.h"

#define WIFI_STA_REQ_CONNECT 0
#define WIFI_STA_CONNECTING 1
#define WIFI_STA_CONNECTED 2

uint8_t wStateNum = WIFI_STA_REQ_CONNECT;
uint wConnCnt = 0;
unsigned long wifiStaScanTime = 0;
String ssid = "";
String pwd = "";
bool isUpdate = false;

void transferConnectState(bool _isConnected, bool _isUpdate = false) {
  wifi_evt_t evtData = {
    .typeNum = WIFI_CHANGE_CONNECT,
    .isConnected = _isConnected,
    .isUpdate = _isUpdate
  };
  xQueueSend(wifi_queue, (void*)&evtData, 10 / portTICK_RATE_MS);
}

MyWiFi::MyWiFi() {
}


void MyWiFi::init() {
  ssid = wifiSSID;
  pwd = wifiPwd;
}

void MyWiFi::renewalData(String _ssid, String _pwd) {
  if (WiFi.isConnected()) {
    WiFi.disconnect();
  }
  transferConnectState(false);
  ssid = _ssid;
  pwd = _pwd;
  isUpdate = true;
  wStateNum = WIFI_STA_REQ_CONNECT;
}

void MyWiFi::sync() {
  if (ssid.length() == 0) {
    return;
  }

  if (millis() - wifiStaScanTime > WIFI_STA_SCAN_TIMER) {
    if (wStateNum == WIFI_STA_REQ_CONNECT) {
      WiFi.begin(ssid.c_str(), pwd.c_str());
      wConnCnt = 0;
      wStateNum = WIFI_STA_CONNECTING;
    } else if (wStateNum == WIFI_STA_CONNECTING) {
      if (WiFi.status() == WL_CONNECTED) {
        wConnCnt = 0;
        wStateNum = WIFI_STA_CONNECTED;
        transferConnectState(true, isUpdate);
        isUpdate = false;
      } else {
        wConnCnt++;
        if (wConnCnt > 5) {
          transferConnectState(false, isUpdate);
          isUpdate = false;
          wStateNum = WIFI_STA_REQ_CONNECT;
        }
      }
    } else if (wStateNum == WIFI_STA_CONNECTED) {
      if (WiFi.status() != WL_CONNECTED) {
        wStateNum = WIFI_STA_REQ_CONNECT;
        transferConnectState(false);
      }
    }
    wifiStaScanTime = millis();
  }
}
