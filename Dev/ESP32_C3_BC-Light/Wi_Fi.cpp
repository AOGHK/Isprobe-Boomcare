#include "Wi_Fi.h"

Wi_Fi mWiFi;

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
#if DEBUG_LOG
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
#endif
}
#pragma endregion

const char* HOST = "asia-northeast2-dadadak-f5f84.cloudfunctions.net";
const char* API_PING_URL = "/pingAPI";
const char* API_THERMO_URL = "/temperatureAPI";

// const char* API_PING_URL = "https://asia-northeast2-dadadak-f5f84.cloudfunctions.net/pingAPI";
// const char* API_THERMO_URL = "https://asia-northeast2-dadadak-f5f84.cloudfunctions.net/temperatureAPI";

// const char* API_PING_URL = "https://3.35.55.75:3001/pingAPI";
// const char* API_THERMO_URL = "https://3.35.55.75:3001/temperatureAPI";
// const char* API_PING_URL = "https://192.168.219.46:3001/pingAPI";
// const char* API_THERMO_URL = "https://192.168.219.46:3001/temperatureAPI";

String mSSID = "";
String mPWD = "";
uint8_t wStaNum = 0;
uint8_t wConnCnt = 0;
bool isWiFiRenewal = false;
bool isRelayConnected = false;

unsigned long syncStaTime = 0;

xQueueHandle wifiConnQueue = xQueueCreate(2, sizeof(bool));
xQueueHandle httpQueue = xQueueCreate(2, sizeof(http_params_t));

thermo_data_t backupThermos[10] = {};
uint8_t backupThermoSize = 0;
uint8_t backupThermoCnt = 0;

// WiFiClientSecure* client = new WiFiClientSecure;

void addBackupThermoItem(thermo_data_t _data) {
  backupThermoCnt++;
  if (backupThermoCnt <= 10) {
    backupThermoSize = backupThermoCnt;
  }
  uint8_t idx = backupThermoCnt % 10;
  idx = idx == 0 ? 9 : idx - 1;
  backupThermos[idx] = _data;
  Rom.addBackupThermo(&_data, idx, backupThermoSize);
}

void clearBackupThermoItems() {
  Rom.clearBakcupThermos(backupThermoSize);
  backupThermoSize = 0;
  backupThermos[10] = {};
}

String getBackupThermoParams(String _addr) {
  char tmBuf[17];
  String paramStr = "{\"data\" : [";
  for (uint8_t i = 0; i < backupThermoSize; i++) {
    sprintf(tmBuf, "%d-%02d-%02d %02d:%02d:%02d",
            backupThermos[i].time[0], backupThermos[i].time[1], backupThermos[i].time[2],
            backupThermos[i].time[3], backupThermos[i].time[4], backupThermos[i].time[5]);
    paramStr += "{\"mac\":\"" + _addr
                + "\", \"temp\":\"" + String(backupThermos[i].val[0]) + "." + String(backupThermos[i].val[1])
                + "\", \"time\":\"" + String(tmBuf) + "\"}";
    if (i != backupThermoSize - 1) {
      paramStr += ",";
    }
  }
  paramStr += "]}";
  return paramStr;
}

bool requestThermoAPI(String paramStr) {
  bool isSuccess = false;

  // WiFiClientSecure client;
  // WiFiClientSecure* client = new WiFiClientSecure;
  // client->setInsecure();

  //   if (client->connect(HOST, 443)) {
  //     client->print(String("POST ") + API_THERMO_URL + " HTTP/1.1\r\n"
  //                   + "Host: " + HOST + "\r\n"
  //                   + "Authorization: key=KpgYkGSogOhtCz6c7kQwB0ETv2k1\r\n"
  //                   + "Content-Type: application/json\r\n"
  //                   + "Content-Length: " + paramStr.length() + "\r\n" + "\r\n" + paramStr + "\n");
  //     while (client->connected()) {
  //       String line = client->readStringUntil('\n');
  //       if (line.startsWith("HTTP/1.1")) {
  // #if DEBUG_LOG
  //         Serial.printf("[WiFi] :: Thermo API Result - ");
  //         Serial.println(line);
  // #endif
  //         isSuccess = line.equals("HTTP/1.1 200 OK");
  //         break;
  //       }
  //     }
  //   }
  //   client->stop();
  // delete client;

  return isSuccess;
}

void requestsThermoAPI(String _addr, thermo_data_t _data) {
  //   char tmBuf[17];
  //   sprintf(tmBuf, "%d-%02d-%02d %02d:%02d:%02d",
  //           _data.time[0], _data.time[1], _data.time[2], _data.time[3], _data.time[4], _data.time[5]);
  //   String paramStr = "{\"data\" : [{\"mac\":\"" + _addr
  //                     + "\", \"temp\":\"" + String(_data.val[0]) + "." + String(_data.val[1])
  //                     + "\", \"time\":\"" + String(tmBuf) + "\"}]}";
  // #if DEBUG_LOG
  //   Serial.printf("[WiFi] :: Thermo API Params -");
  //   Serial.println(paramStr);
  // #endif
  //   if (requestThermoAPI(paramStr)) {
  //     if (backupThermoSize != 0) {
  //       paramStr = getBackupThermoParams(_addr);
  //       if (requestThermoAPI(paramStr)) {
  //         clearBackupThermoItems();
  //       }
  //     }
  //   } else {
  //     addBackupThermoItem(_data);
  //   }

  HTTPClient http;
  char tmBuf[17];
  sprintf(tmBuf, "%d-%02d-%02d %02d:%02d:%02d",
          _data.time[0], _data.time[1], _data.time[2], _data.time[3], _data.time[4], _data.time[5]);
  String paramStr = "{\"data\" : [{\"mac\":\"" + _addr
                    + "\", \"temp\":\"" + String(_data.val[0]) + "." + String(_data.val[1])
                    + "\", \"time\":\"" + String(tmBuf) + "\"}]}";
#if DEBUG_LOG
  Serial.printf("[WiFi] :: Thermo API Params -");
  Serial.println(paramStr);
#endif
  http.setConnectTimeout(500);
  http.setTimeout(500);
  if (http.begin(API_THERMO_URL)) {
    http.addHeader("Content-Type", "application/json");
    int resCode = http.POST(paramStr);
#if DEBUG_LOG
    Serial.printf("[WiFi] :: Thermo API Res Code -");
    Serial.println(resCode);
#endif
    if (resCode == 200) {
      if (backupThermoSize != 0) {
        paramStr = getBackupThermoParams(_addr);
        resCode = http.POST(paramStr);
        if (resCode == 200) {
          clearBackupThermoItems();
        }
      }
    } else {
      addBackupThermoItem(_data);
    }
  }
  http.end();

  //   HTTPClient http;
  //   char tmBuf[17];
  //   sprintf(tmBuf, "%d-%02d-%02d+%02d:%02d:%02d",
  //           _data.time[0], _data.time[1], _data.time[2], _data.time[3], _data.time[4], _data.time[5]);
  //   String url = String(API_THERMO_URL) + "mac=" + _addr
  //                + "&temp=" + String(_data.val[0]) + "." + String(_data.val[1])
  //                + "&time=" + String(tmBuf);
  // #if DEBUG_LOG
  //   Serial.println(url);
  // #endif
  //   http.setConnectTimeout(2000);
  //   http.setTimeout(2000);
  //   if (http.begin(url)) {
  //     int resCode = http.GET();
  // #if DEBUG_LOG
  //     Serial.printf("[WiFi] :: Thermo API Res Code - ");
  //     Serial.println(resCode);
  // #endif
  //   }
}

void requsetPingApi(String _addr, uint8_t _batLvl) {
  String paramStr = "{\"data\" : [{\"mac\":\"" + _addr
                    + "\", \"bat_lvl\":\"" + String(_batLvl) + "\"}]}";
#if DEBUG_LOG
  Serial.printf("[WiFi] :: Ping API Params - ");
  Serial.println(paramStr);
#endif
  // WiFiClientSecure client;
  WiFiClientSecure* client = new WiFiClientSecure;
  // client->setCACert(root_ca);
  client->setInsecure();
  if (!client->connect(HOST, 443)) {
    return;
  }

  client->print(String("POST ") + API_PING_URL + " HTTP/1.1\r\n"
                + "Host: " + HOST + "\r\n"
                + "Authorization: key=KpgYkGSogOhtCz6c7kQwB0ETv2k1\r\n"
                + "Content-Type: application/json\r\n"
                + "Content-Length: " + paramStr.length() + "\r\n" + "\r\n" + paramStr + "\n");

  while (client->connected()) {
    String line = client->readStringUntil('\n');
    if (line.startsWith("HTTP/1.1")) {
#if DEBUG_LOG
      Serial.printf("[WiFi] :: Ping API Result - ");
      Serial.println(line);
#endif
      break;
    }
  }
  client->stop();
  delete client;

  //   HTTPClient http;
  //   String paramStr = "{\"mac\":\"" + _addr
  //                     + "\", \"bat_lvl\":\"" + String(_batLvl) + "\"}";
  // #if DEBUG_LOG
  //   Serial.printf("[WiFi] :: Ping API Params -");
  //   Serial.println(paramStr);
  // #endif
  //   http.setConnectTimeout(500);
  //   http.setTimeout(500);

  //   if (http.begin(API_PING_URL)) {
  //     http.addHeader("Content-Type", "application/json");
  //     int resCode = http.POST(paramStr);
  // #if DEBUG_LOG
  //     Serial.printf("[WiFi] :: Ping API Res Code -");
  //     Serial.println(resCode);
  // #endif
  //   }
  //   http.end();
}

void transferWiFiResult(bool _isConn) {
#if DEBUG_LOG
  Serial.printf("[WiFi] :: Connected - %d\n", _isConn);
#endif
  if (!isRelayConnected) { return; }
  xQueueSend(wifiConnQueue, (void*)&_isConn, 1 / portTICK_RATE_MS);
  isRelayConnected = false;
}

uint8_t getWiFiState() {
  uint8_t staNum = WiFi.status();
  if (staNum == 255) {  //WL_NO_SHIELD
    WiFi.begin(mSSID.c_str(), mPWD.c_str());
    return WIFI_BEGIN_CONNECT;
  } else if (staNum == 3) {  //WL_CONNECTED
    wConnCnt = 0;
    return WIFI_STA_CONNECTED;
  } else if (wConnCnt == 10) {
    return WIFI_STA_CONNECT_FAIL;
  } else {
    wConnCnt = wConnCnt < 20 ? wConnCnt++ : 20;
    return WIFI_STA_DISCONNECT;
  }
}

void checkWiFiConnection() {
  if (millis() - syncStaTime < WIFI_STA_SYNC_TIMER) {
    return;
  }
  syncStaTime = millis();
  uint8_t staNum = getWiFiState();
  if (wStaNum == staNum) {
    return;
  }
  wStaNum = staNum;
  if (wStaNum == WIFI_STA_CONNECTED) {
    syncNTPTime();
    if (isWiFiRenewal) {  // Renewal WiFi Info.
      Rom.setWiFi(mSSID, mPWD);
      isWiFiRenewal = false;
    }
#if DEBUG_LOG
    Serial.print("[WiFi] IP address : ");
    Serial.println(WiFi.localIP());
#endif
    transferWiFiResult(true);
  } else if (wStaNum == WIFI_STA_CONNECT_FAIL) {
    transferWiFiResult(false);
  }
}

void taskWiFiClient(void* param) {
  while (1) {
    http_params_t _params;
    if (xQueueReceive(httpQueue, &_params, 1 / portTICK_RATE_MS)) {
      if (_params.type == HTTP_PING_API) {
        requsetPingApi(_params.macAddress, _params.batLevel);
      } else if (_params.type == HTTP_THERMO_API) {
        requestsThermoAPI(_params.macAddress, _params.thermo);
      }
    }
    if (mSSID.length() == 0) {
      vTaskDelay(50 / portTICK_RATE_MS);
      continue;
    }
    checkWiFiConnection();
    vTaskDelay(10 / portTICK_RATE_MS);
  }
}


Wi_Fi::Wi_Fi() {
}

void Wi_Fi::begin() {
  Rom.getWiFi(&mSSID, &mPWD);
#if DEBUG_LOG
  Serial.printf("[WiFi] :: Init SSID(%s), Pwd(%s)\n", mSSID.c_str(), mPWD.c_str());
#endif

  Rom.getBackupThermos(backupThermos, &backupThermoSize);
  backupThermoCnt = backupThermoSize;

  // client->setInsecure();
  // client->setCACert(root_ca);
  // client->setTimeout(500);

  // #if DEBUG_LOG
  //   Serial.printf("[WiFi] :: Backup Thermo Size - %d\n", backupThermoSize);
  //   char tmBuf[17];
  //   for (uint8_t i = 0; i < backupThermoSize; i++) {
  //     sprintf(tmBuf, "%02d-%02d-%02d %02d:%02d:%02d",
  //             backupThermos[i].time[0], backupThermos[i].time[1], backupThermos[i].time[2],
  //             backupThermos[i].time[3], backupThermos[i].time[4], backupThermos[i].time[5]);
  //     String thermo = String(backupThermos[i].val[0]) + "." + String(backupThermos[i].val[1]);
  //     Serial.printf("Item %d -> %s, %s\n", i, tmBuf, thermo.c_str());
  //   }
  //   Serial.println();
  // #endif

  xTaskCreate(taskWiFiClient, "WIFI_CLIENT_TASK", 1024 * 16, NULL, 3, NULL);
}

bool Wi_Fi::isConnected() {
  return wStaNum == WIFI_STA_CONNECTED;
}

void Wi_Fi::renewalData(String _data) {
  uint8_t cutIdx = _data.indexOf(',');
  mSSID = _data.substring(0, cutIdx);
  mPWD = _data.substring(cutIdx + 1, _data.length());
#if DEBUG_LOG
  Serial.printf("[WiFi] :: Renewal SSID(%s), Pwd(%s)\n", mSSID.c_str(), mPWD.c_str());
#endif
  WiFi.begin(mSSID.c_str(), mPWD.c_str());
  wStaNum = WIFI_BEGIN_CONNECT;
  isWiFiRenewal = true;
  isRelayConnected = true;
}

void Wi_Fi::syncPing(String _addr, uint8_t _batLevel) {
  if (wStaNum != WIFI_STA_CONNECTED) {
    return;
  }
  http_params_t param = {
    .type = HTTP_PING_API,
    .batLevel = _batLevel,
  };
  _addr.toCharArray(param.macAddress, 18);
  xQueueSend(httpQueue, (void*)&param, 1 / portTICK_RATE_MS);
}

void Wi_Fi::uploadThermo(String _addr, uint16_t _value) {
  if (wStaNum != WIFI_STA_CONNECTED) {
    return;
  }

  http_params_t param = {
    .type = HTTP_THERMO_API,
  };
  _addr.toCharArray(param.macAddress, 18);

  param.thermo.val[0] = _value / 100;
  param.thermo.val[1] = _value % 100;

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return;
  }
  param.thermo.time[0] = (timeinfo.tm_year + 1900);
  param.thermo.time[1] = timeinfo.tm_mon + 1;
  param.thermo.time[2] = timeinfo.tm_mday;
  param.thermo.time[3] = timeinfo.tm_hour;
  param.thermo.time[4] = timeinfo.tm_min;
  param.thermo.time[5] = timeinfo.tm_sec;

  xQueueSend(httpQueue, (void*)&param, 1 / portTICK_RATE_MS);
}