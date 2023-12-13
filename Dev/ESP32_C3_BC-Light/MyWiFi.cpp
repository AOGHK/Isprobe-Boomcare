#include "MyWiFi.h"
#include "BLE.h"

#pragma region NTP Time Func
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
extern uint8_t BATTERY_LVL;

bool isNewTmpData = false;
tmp_param_t newTmpData;
tmp_param_t errTmpData[10] = {};
uint8_t errCnt = 0;

uint8_t staNum = WIFI_STA_REQ_CONNECT;
uint8_t wConnCnt = 0;
bool isConnRenewal = false;
unsigned long syncStaTime = 0;
unsigned long reqPingTime = 0;

void (*_stateCallback)(bool, bool);

void MyWiFi::setCallback(void (*stateCallback)(bool, bool)) {
  _stateCallback = stateCallback;
}

void submitChangeState(bool _isConnected, bool _isConnRenewal) {
  if (_stateCallback != nullptr) {
    _stateCallback(_isConnected, _isConnRenewal);
  }
}

/*  -------------------------------------------------
                  Error Data Func
------------------------------------------------- */
void addErrorThermoValue() {
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

void clearErrorThermoValues() {
  uint8_t endPos = (errCnt - 1) * 8 + 26;
  for (uint8_t i = 18; i < endPos; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
  errCnt = 0;
  errTmpData[10] = {};
}

/*  -------------------------------------------------
                  REST API Func
------------------------------------------------- */
void requestThermoAPI() {
  HTTPClient http;

  char tmBuf[17];
  sprintf(tmBuf, "%02d-%02d-%02d %02d:%02d:%02d",
          newTmpData.time[0], newTmpData.time[1], newTmpData.time[2], newTmpData.time[3], newTmpData.time[4], newTmpData.time[5]);
  String nParamsStr = "{\"data\" : [{\"mac\":\"" + MY_MAC_ADDRESS
                      + "\", \"temp\":\"" + String(newTmpData.tmp[0]) + "." + String(newTmpData.tmp[1])
                      + "\", \"time\":\"" + String(tmBuf) + "\"}]}";

  http.setConnectTimeout(500);
  http.setTimeout(500);
  if (http.begin(API_TMP_URL)) {
    http.addHeader("Content-Type", "application/json");
    int resCode = http.POST(nParamsStr);
    if (resCode == 200) {
      if (errCnt != 0) {
        String eParamsStr = "{\"data\" : [";
        for (uint8_t i = 0; i < errCnt; i++) {
          sprintf(tmBuf, "%02d-%02d-%02d %02d:%02d:%02d",
                  errTmpData[i].time[0], errTmpData[i].time[1], errTmpData[i].time[2],
                  errTmpData[i].time[3], errTmpData[i].time[4], errTmpData[i].time[5]);
          eParamsStr += "{\"mac\":\"" + MY_MAC_ADDRESS
                        + "\", \"temp\":\"" + String(errTmpData[i].tmp[0]) + "." + String(errTmpData[i].tmp[1])
                        + "\", \"time\":\"" + String(tmBuf) + "\"}";
          if (i != errCnt - 1) {
            eParamsStr += ",";
          }
        }
        eParamsStr += "]}";
        resCode = http.POST(eParamsStr);
        if (resCode == 200) {
          clearErrorThermoValues();
        }
      }
    } else {
      addErrorThermoValue();
    }
  }
  http.end();
}

void requestPingAPI() {
  HTTPClient http;

  String params_Str = "{\"mac\":\"" + MY_MAC_ADDRESS
                      + "\", \"bat_lvl\":\"" + String(BATTERY_LVL) + "\"}";

  http.setConnectTimeout(500);
  http.setTimeout(500);
  if (http.begin(API_PING_URL)) {
    http.addHeader("Content-Type", "application/json");
    int resCode = http.POST(params_Str);
  }
  http.end();
}

void syncConnectState() {
  if (millis() - syncStaTime > WIFI_STA_SYNC_TIMER) {
    if (staNum == WIFI_STA_REQ_CONNECT) {
      if (WiFi.isConnected()) {
        WiFi.disconnect();
        submitChangeState(false, false);
      }
      wConnCnt = 0;
      WiFi.begin(mSSID.c_str(), mPWD.c_str());
      staNum = WIFI_STA_CONNECTING;
    } else if (staNum == WIFI_STA_CONNECTING) {
      if (WiFi.status() == WL_CONNECTED) {
        staNum = WIFI_STA_CONNECTED;
        syncNTPTime();
        submitChangeState(true, isConnRenewal);
        isConnRenewal = false;
      } else if (wConnCnt > WIFI_CONNECT_ERR_CNT) {
        staNum = WIFI_STA_REQ_CONNECT;
        submitChangeState(false, isConnRenewal);
        isConnRenewal = false;
      } else {
        wConnCnt++;
      }
    } else if (staNum == WIFI_STA_CONNECTED) {
      if (WiFi.status() != WL_CONNECTED) {
        staNum = WIFI_STA_REQ_CONNECT;
        submitChangeState(false, false);
      }
    }
    syncStaTime = millis();
  }
}

void taskWiFiClient(void* param) {
  while (1) {
    if (mSSID.length() != 0) {
      syncConnectState();

      if (isNewTmpData) {
        if (staNum == WIFI_STA_CONNECTED) {
          requestThermoAPI();
        } else {
          addErrorThermoValue();
        }
        isNewTmpData = false;
      }

      if (staNum == WIFI_STA_CONNECTED) {
        if (millis() - reqPingTime > WIFI_PING_TIMER) {
          requestPingAPI();
          reqPingTime = millis();
        }
      }
      vTaskDelay(10 / portTICK_RATE_MS);
    } else {
      vTaskDelay(50 / portTICK_RATE_MS);
    }
  }
}

MyWiFi::MyWiFi() {
}

void MyWiFi::begin() {
  readRom();
  // xTaskCreatePinnedToCore(taskWiFiClient, "WiFi_CLIENT_TASK", 1024 * 8, NULL, 3, NULL, 0);
  xTaskCreate(taskWiFiClient, "WiFi_CLIENT_TASK", 1024 * 8, NULL, 3, NULL);
}

void MyWiFi::renewalConnect(String _SSID, String _Pwd) {
  mSSID = _SSID;
  mPWD = _Pwd;
  isConnRenewal = true;
  staNum = WIFI_STA_REQ_CONNECT;
}

void MyWiFi::uploadThermoValue(uint16_t _value) {
  if (mSSID.length() == 0) {
    return;
  }

  isNewTmpData = true;

  newTmpData.tmp[0] = _value / 100;
  newTmpData.tmp[1] = _value % 100;

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return;
  }
  newTmpData.time[0] = (timeinfo.tm_year + 1900) % 100;
  newTmpData.time[1] = timeinfo.tm_mon + 1;
  newTmpData.time[2] = timeinfo.tm_mday;
  newTmpData.time[3] = timeinfo.tm_hour;
  newTmpData.time[4] = timeinfo.tm_min;
  newTmpData.time[5] = timeinfo.tm_sec;
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

#if DEBUG_LOG
  Serial.print("[ROM] :: Saved WiFi - ");
  Serial.print(mSSID);
  Serial.print(", ");
  Serial.println(mPWD);
#endif
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
