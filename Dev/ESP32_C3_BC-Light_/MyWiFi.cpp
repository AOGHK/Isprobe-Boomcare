#include "MyWiFi.h"

MyWiFi mWiFi;

#pragma region>> NTP Time Func
const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
uint8_t timeZone = 9;
uint8_t summerTime = 0;  // 3600
time_t nTime;

void syncNTPTime() {
  configTime(3600 * timeZone, 3600 * summerTime, ntpServer1, ntpServer2);
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return;
  }
  uint8_t yy = (timeinfo.tm_year + 1900) % 100;
  uint8_t MM = timeinfo.tm_mon + 1;
  uint8_t dd = timeinfo.tm_mday;
  uint8_t hh = timeinfo.tm_hour;
  uint8_t mm = timeinfo.tm_min;
  uint8_t ss = timeinfo.tm_sec;
}
#pragma endregion

const char* API_PING_URL = "https://192.168.219.108:3000/light/ping";
const char* API_TMP_URL = "https://192.168.219.108:3000/light/temperature";

String mSSID = "";
String mPWD = "";
uint8_t staNum = WIFI_STA_REQ_CONNECT;
uint8_t wConnCnt = 0;
bool isConnRenewal = false;
tmp_param_t prevTmpData[10] = {};
uint8_t prevTmpSize = 0;
unsigned long syncStaTime = 0;


void syncConnectState() {
  if (millis() - syncStaTime > WIFI_STA_SYNC_TIMER) {
    if (staNum == WIFI_STA_REQ_CONNECT) {
      if (WiFi.isConnected()) {
        WiFi.disconnect();
        // + LED -> WiFi Disconnect
      }
      wConnCnt = 0;
      WiFi.begin(mSSID.c_str(), mPWD.c_str());
      staNum = WIFI_STA_CONNECTING;
    } else if (staNum == WIFI_STA_CONNECTING) {
      if (WiFi.status() == WL_CONNECTED) {
        staNum = WIFI_STA_CONNECTED;
        syncNTPTime();
        // + LED -> WiFi Disconnect
        // + ROM -> Save WiFi
      } else if (wConnCnt > WIFI_CONNECT_ERR_CNT) {
        staNum = WIFI_STA_REQ_CONNECT;
        // + LED -> WiFi Disconnect
      } else {
        wConnCnt++;
      }
    } else if (staNum == WIFI_STA_CONNECTED) {
      if (WiFi.status() != WL_CONNECTED) {
        staNum = WIFI_STA_REQ_CONNECT;
        // + LED -> WiFi Disconnect
      }
    }
    syncStaTime = millis();
  }
}

void taskWiFiClient(void* param) {
  while (1) {
    if (mSSID.length() != 0) {
      syncConnectState();

      // + API -> Temp
      // + API -> Ping

      vTaskDelay(10 / portTICK_RATE_MS);
    } else {
      vTaskDelay(50 / portTICK_RATE_MS);
    }
  }
}

MyWiFi::MyWiFi() {
}

void MyWiFi::begin() {
  ROM.getWiFi(&mSSID, &mPWD);
#if DEBUG_LOG
  Serial.print("[WiFi] :: Connect Info - ");
  Serial.print(mSSID);
  Serial.print(", ");
  Serial.println(mPWD);
#endif

  xTaskCreate(taskWiFiClient, "WiFi_CLIENT_TASK", 1024 * 8, NULL, 3, NULL);
}
