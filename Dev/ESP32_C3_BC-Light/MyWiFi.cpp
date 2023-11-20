#include "MyWiFi.h"

extern uint8_t batLevel;
extern String myMacAddress;

#pragma region NTP Time Func
const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
uint8_t timeZone = 9;
uint8_t summerTime = 0;  // 3600
time_t nTime;

String getCurrentTime() {
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

  return timeStr;
}

void setCurrentTime(uint8_t* timeArr) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return;
  }
  timeArr[0] = (timeinfo.tm_year + 1900) % 100;
  timeArr[1] = timeinfo.tm_mon + 1;
  timeArr[2] = timeinfo.tm_mday;
  timeArr[3] = timeinfo.tm_hour;
  timeArr[4] = timeinfo.tm_min;
  timeArr[5] = timeinfo.tm_sec;
}

String getTimeStr(uint8_t* timeArr) {
  char buf[17];
  sprintf(buf, "%02d-%02d-%02d %02d:%02d:%02d",
          timeArr[0], timeArr[1], timeArr[2], timeArr[3], timeArr[4], timeArr[5]);
  return String(buf);
}

void syncNTPTime() {
  configTime(3600 * timeZone, 3600 * summerTime, ntpServer1, ntpServer2);
  getCurrentTime();
}
#pragma endregion

const char* API_PING_URL = "https://192.168.219.106:3000/light/ping";
const char* API_TMP_URL = "https://192.168.219.106:3000/light/temperature";

String mSSID = "";
String mPWD = "";

uint8_t staNum = WIFI_STA_REQ_CONNECT;
uint8_t connCnt = 0;
bool isRenewal = false;

unsigned long connScanTime = 0;
unsigned long pingTime = 0;

bool isNewTemperature = false;
tmp_param_t newTmpData;
tmp_param_t errTmpData[10] = {};
uint8_t errCnt = 0;

void (*_connectCallback)(bool, bool);
void MyWiFi::setConnectCallback(void (*evtCallback)(bool, bool)) {
  _connectCallback = evtCallback;
}

MyWiFi::MyWiFi() {
}

void saveTmpErrorData() {
  uint8_t eCnt = errCnt + 1;
  if (eCnt <= 10) {
    errCnt = eCnt;
    EEPROM.write(18, eCnt);
  }
  uint8_t idx = eCnt % 10;
  if (idx == 0) {
    idx = 10 - 1;
  } else {
    idx -= 1;
  }
  errTmpData[idx] = newTmpData;
  uint8_t addrPos = idx * 8 + 19;
  EEPROM.write(addrPos, errTmpData[idx].time[0]);
  EEPROM.write(addrPos + 1, errTmpData[idx].time[1]);
  EEPROM.write(addrPos + 2, errTmpData[idx].time[2]);
  EEPROM.write(addrPos + 3, errTmpData[idx].time[3]);
  EEPROM.write(addrPos + 4, errTmpData[idx].time[4]);
  EEPROM.write(addrPos + 5, errTmpData[idx].time[5]);
  EEPROM.write(addrPos + 6, errTmpData[idx].tmp[0]);
  EEPROM.write(addrPos + 7, errTmpData[idx].tmp[1]);
  EEPROM.commit();
}

void clearTmpErrorData() {
  uint8_t endPos = (errCnt - 1) * 8 + 26;
  for (uint8_t i = 18; i < endPos; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
  errCnt = 0;
  errTmpData[10] = {};
}

void sendTemperatureAPI() {
  HTTPClient http;

  String nParamsStr = "{\"data\" : [{\"mac\":\"" + myMacAddress
                      + "\", \"temp\":\"" + String(newTmpData.tmp[0]) + "." + String(newTmpData.tmp[1])
                      + "\", \"time\":\"" + getTimeStr(newTmpData.time) + "\"}]}";

  http.setConnectTimeout(1000);
  http.setTimeout(1000);
  if (http.begin(API_TMP_URL)) {
    http.addHeader("Content-Type", "application/json");
    int resCode = http.POST(nParamsStr);
    if (resCode == 200) {
      if (errCnt != 0) {
        String eParamsStr = "{\"data\" : [";
        for (uint8_t i = 0; i < errCnt; i++) {
          eParamsStr += "{\"mac\":\"" + myMacAddress
                        + "\", \"temp\":\"" + String(errTmpData[i].tmp[0]) + "." + String(errTmpData[i].tmp[1])
                        + "\", \"time\":\"" + getTimeStr(errTmpData[i].time) + "\"}";
          if (i != errCnt - 1) {
            eParamsStr += ",";
          }
        }
        eParamsStr += "]}";
        resCode = http.POST(eParamsStr);
        if (resCode == 200) {
          clearTmpErrorData();
        }
      }
    } else {
      saveTmpErrorData();
    }
  }
  http.end();
}


void sendPingAPI() {
  HTTPClient http;

  String params_Str = "{\"mac\":\"" + myMacAddress
                      + "\", \"bat_lvl\":\"" + String(batLevel) + "\"}";

  http.setConnectTimeout(1000);
  http.setTimeout(1000);
  if (http.begin(API_PING_URL)) {
    http.addHeader("Content-Type", "application/json");
    int resCode = http.POST(params_Str);
  }
  http.end();
}

void checkConnectTimer() {
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

void taskWiFiClient(void* param) {
  while (1) {
    if (mSSID.length() != 0) {
      checkConnectTimer();

      if (isNewTemperature) {
        if (staNum == WIFI_STA_CONNECTED) {
          sendTemperatureAPI();
        } else {
          saveTmpErrorData();
        }
        isNewTemperature = false;
      }

      if (staNum == WIFI_STA_CONNECTED) {
        if (millis() - pingTime > WIFI_PING_TIMER) {
          sendPingAPI();
          pingTime = millis();
        }
      }
      vTaskDelay(10 / portTICK_RATE_MS);
    } else {
      vTaskDelay(50 / portTICK_RATE_MS);
    }
  }
}

void MyWiFi::begin() {
  readRom();

  xTaskCreatePinnedToCore(taskWiFiClient, "WiFi_CLIENT_TASK", 1024 * 8, NULL, 1, NULL, 1);
}

void MyWiFi::readRom() {
  uint8_t nameLen = EEPROM.read(99);
  uint8_t pwdLen = EEPROM.read(100);
  if (nameLen != 0) {
    for (uint8_t idx = 0; idx < nameLen + pwdLen; idx++) {
      if (idx < nameLen) {
        mSSID += (char)EEPROM.read(idx + 101);
      } else {
        mPWD += (char)EEPROM.read(idx + 101);
      }
    }
  }
  errCnt = EEPROM.read(18);
  for (uint8_t i = 0; i < errCnt; i++) {
    uint8_t addrPos = i * 8 + 19;
    errTmpData[i].time[0] = EEPROM.read(addrPos);
    errTmpData[i].time[1] = EEPROM.read(addrPos + 1);
    errTmpData[i].time[2] = EEPROM.read(addrPos + 2);
    errTmpData[i].time[3] = EEPROM.read(addrPos + 3);
    errTmpData[i].time[4] = EEPROM.read(addrPos + 4);
    errTmpData[i].time[5] = EEPROM.read(addrPos + 5);
    errTmpData[i].tmp[0] = EEPROM.read(addrPos + 6);
    errTmpData[i].tmp[1] = EEPROM.read(addrPos + 7);
  }
}

void MyWiFi::updateRom() {
  uint8_t nameLen = mSSID.length();
  uint8_t pwdLen = mPWD.length();
  EEPROM.write(99, nameLen);
  EEPROM.write(100, pwdLen);
  for (uint8_t i = 0; i < nameLen + pwdLen; i++) {
    if (i < nameLen) {
      EEPROM.write(i + 101, mSSID[i]);
    } else {
      EEPROM.write(i + 101, mPWD[i - nameLen]);
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

void MyWiFi::uploadTemperature(uint16_t _value) {
  isNewTemperature = true;
  newTmpData.tmp[0] = _value / 100;
  newTmpData.tmp[1] = _value % 100;
  setCurrentTime(newTmpData.time);
}
