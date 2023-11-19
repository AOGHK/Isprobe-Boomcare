#include "MyWiFi.h"


const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
uint8_t timeZone = 9;
uint8_t summerTime = 0;  // 3600
time_t nTime;

String MyWiFi::getCurrentTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Unknown";
  }
  uint8_t yy = (timeinfo.tm_year + 1900) % 100;
  uint8_t MM = timeinfo.tm_mon + 1;
  uint8_t dd = timeinfo.tm_mday;
  uint8_t hh = timeinfo.tm_hour;
  uint8_t mm = timeinfo.tm_min;
  uint8_t ss = timeinfo.tm_sec;
  String timeStr = String(yy) + "-" + String(MM) + "-" + String(dd) + " "
                   + String(hh) + ":" + String(mm) + "-" + String(ss);

  Serial.print("KST Time : ");
  Serial.println(timeStr);
  return timeStr;
}

void MyWiFi::syncNTPTime() {
  configTime(3600 * timeZone, 3600 * summerTime, ntpServer1, ntpServer2);
  getCurrentTime();
}

String mSSID = "";
String mPWD = "";

unsigned long connScanTime = 0;
uint8_t staNum = WIFI_STA_REQ_CONNECT;
uint8_t connCnt = 0;
bool isRenewal = false;

void (*_connectCallback)(bool, bool);
void MyWiFi::setConnectCallback(void (*evtCallback)(bool, bool)) {
  _connectCallback = evtCallback;
}
MyWiFi::MyWiFi() {
}

void MyWiFi::begin() {
  readRom();
}

void MyWiFi::scanConnected() {
  if (millis() - connScanTime > WIFI_STA_SCAN_TIMER) {
    if (staNum == WIFI_STA_REQ_CONNECT) {
      if (WiFi.isConnected()) {
        WiFi.disconnect();
        if (_connectCallback != nullptr) {
          _connectCallback(false, false);
        }
      }
      connCnt = 0;
      WiFi.begin(mSSID.c_str(), mPWD.c_str());
      staNum = WIFI_STA_CONNECTING;
    } else if (staNum == WIFI_STA_CONNECTING) {
      if (WiFi.status() == WL_CONNECTED) {
        staNum = WIFI_STA_CONNECTED;
        syncNTPTime();
        if (_connectCallback != nullptr) {
          _connectCallback(true, isRenewal);
        }
        isRenewal = false;
      } else if (connCnt > WIFI_CONNECT_ERR_CNT) {
        staNum = WIFI_STA_REQ_CONNECT;
        if (_connectCallback != nullptr) {
          _connectCallback(false, isRenewal);
        }
        isRenewal = false;
      } else {
        connCnt++;
      }
    } else if (staNum == WIFI_STA_CONNECTED) {
      if (WiFi.status() != WL_CONNECTED) {
        staNum = WIFI_STA_REQ_CONNECT;
        if (_connectCallback != nullptr) {
          _connectCallback(false, false);
        }
      }
    }
    connScanTime = millis();
  }
}


void MyWiFi::readRom() {
  uint8_t nameLen = EEPROM.read(89);
  uint8_t pwdLen = EEPROM.read(90);
  if (nameLen != 0) {
    for (uint8_t idx = 0; idx < nameLen + pwdLen; idx++) {
      if (idx < nameLen) {
        mSSID += (char)EEPROM.read(idx + 91);
      } else {
        mPWD += (char)EEPROM.read(idx + 91);
      }
    }
  }
  Serial.print("WiFi SSID - ");
  Serial.print(mSSID);
  Serial.print(", PWD - ");
  Serial.println(mPWD);
}

void MyWiFi::updateRom() {
  uint8_t nameLen = mSSID.length();
  uint8_t pwdLen = mPWD.length();
  EEPROM.write(89, nameLen);
  EEPROM.write(90, pwdLen);
  for (uint8_t i = 0; i < nameLen + pwdLen; i++) {
    if (i < nameLen) {
      EEPROM.write(i + 91, mSSID[i]);
    } else {
      EEPROM.write(i + 91, mPWD[i - nameLen]);
    }
  }
  EEPROM.commit();
}

void MyWiFi::renewalData(String _SSID, String _Pwd) {
  mSSID = _SSID;
  mPWD = _Pwd;
  isRenewal = true;
  staNum = WIFI_STA_REQ_CONNECT;
}

void MyWiFi::scanState() {
  if (mSSID.length() == 0) {
    return;
  }
  scanConnected();
}