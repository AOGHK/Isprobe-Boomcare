#include "WIFI.h"

WIFIClass mWiFi;

String mSSID = "";
String mPWD = "";

unsigned long syncStaTime = 0;
uint8_t staNum = 0;
uint8_t connCnt = 0;
bool isWiFiRenewal = false;
bool isAnswerConnect = false;

const char* HTTP_PING_URL = "http://3.35.55.75:3000/boomcare/ping";
const char* HTTP_THERMO_URL = "http://3.35.55.75:3000/boomcare/temperature";

xQueueHandle httpQueue = xQueueCreate(2, sizeof(http_params_t));

#pragma region NTP Time Func
const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
uint8_t timeZone = 9;
uint8_t summerTime = 0;  // 3600

void syncNTPTime() {
  configTime(3600 * timeZone, 3600 * summerTime, ntpServer1, ntpServer2);
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return;
  }
}
#pragma endregion

String getBackupTemperature(String _addr) {
  uint8_t _size = Rom.getTemperatureSize();
  if (_size == 0) {
    return "";
  }

  temp_date_t* _datetime = (temp_date_t*)malloc(_size * sizeof(temp_date_t));
  temp_value_t* _value = (temp_value_t*)malloc(_size * sizeof(temp_value_t));
  Rom.getTemperature(_datetime, _value);

  char dt[17];
  String paramStr = "{\"data\" : [";
  for (uint8_t i = 0; i < _size; i++) {
    sprintf(dt, "%02d-%02d-%02d %02d:%02d:%02d",
            _datetime[i].year, _datetime[i].month, _datetime[i].day,
            _datetime[i].hour, _datetime[i].min, _datetime[i].sec);
    paramStr += "{\"mac\":\"" + _addr
                + "\", \"temp\":\"" + String(_value[i].integer) + "." + String(_value[i].point)
                + "\", \"time\":\"" + String(dt) + "\"}";
    if (i != _size - 1) {
      paramStr += ",";
    }
  }
  paramStr += "]}";
  free(_datetime);
  free(_value);
  return paramStr;
}

void requsetTemperatureApi(http_params_t* _params) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return;
  }
  temp_date_t datetime = {
    .year = (timeinfo.tm_year + 1900) % 100,
    .month = timeinfo.tm_mon + 1,
    .day = timeinfo.tm_mday,
    .hour = timeinfo.tm_hour,
    .min = timeinfo.tm_min,
    .sec = timeinfo.tm_sec
  };

  temp_value_t value = {
    .integer = _params->value / 100,
    .point = _params->value % 100
  };

  char addr[17];
  sprintf(addr, "%02X:%02X:%02X:%02X:%02X:%02X",
          _params->addr[0], _params->addr[1], _params->addr[2],
          _params->addr[3], _params->addr[4], _params->addr[5]);

  char dt[17];
  sprintf(dt, "%02d-%02d-%02d %02d:%02d:%02d",
          datetime.year, datetime.month, datetime.day, datetime.hour, datetime.min, datetime.sec);

  String paramStr = "{\"data\" : [{\"mac\":\"" + String(addr)
                    + "\", \"temp\":\"" + String(value.integer) + "." + String(value.point)
                    + "\", \"time\":\"" + String(dt) + "\"}]}";

  ESP_LOGE(WIFI_TAG, "Request Temperature API -> %s", paramStr.c_str());

  HTTPClient http;
  http.setTimeout(5000);
  if (http.begin(HTTP_THERMO_URL)) {
    http.addHeader("Content-Type", "application/json");
    int16_t resCode = http.POST(paramStr);
    ESP_LOGE(WIFI_TAG, "Temperature Response Code - %d", resCode);
    http.getString();
    if (resCode == 200) {
      paramStr = getBackupTemperature(String(addr));
      if (paramStr.length() != 0) {
        resCode = http.POST(paramStr);
        ESP_LOGE(WIFI_TAG, "Backup Temperature Response Code - %d", resCode);
        http.getString();
        if (resCode == 200) {
          Rom.clearTemperature();
        }
      }
    } else {
      Rom.addTemperature(&datetime, &value);
    }
  }
  http.end();
}

void requsetPingApi(http_params_t* _params) {
  char addr[17];
  sprintf(addr, "%02X:%02X:%02X:%02X:%02X:%02X",
          _params->addr[0], _params->addr[1], _params->addr[2],
          _params->addr[3], _params->addr[4], _params->addr[5]);

  String paramStr = "{\"data\" : [{\"mac\":\"" + String(addr)
                    + "\", \"bat_lvl\":\"" + String(_params->value) + "\"}]}";

  ESP_LOGE(WIFI_TAG, "Request Ping API");

  HTTPClient http;
  http.setTimeout(2000);
  if (http.begin(HTTP_PING_URL)) {
    http.addHeader("Content-Type", "application/json");
    int16_t resCode = http.POST(paramStr);
    ESP_LOGE(WIFI_TAG, "Ping Response Code - %d", resCode);
    http.getString();
  }
  http.end();
}

void sendConnectResult(bool _isConn) {
  ESP_LOGE(WIFI_TAG, "WiFi Connected - %d", _isConn);
  if (!isAnswerConnect) { return; }
  Proc.sendEvtQueue(WIFI_CONNECT_RESULT, _isConn);
  isAnswerConnect = false;
}

uint8_t getWiFiState() {
  uint8_t staNum = WiFi.status();
  if (staNum == 255) {  //WL_NO_SHIELD
    WiFi.begin(mSSID.c_str(), mPWD.c_str());
    return WIFI_BEGIN_CONNECT;
  } else if (staNum == 3) {  //WL_CONNECTED
    connCnt = 0;
    return WIFI_STA_CONNECTED;
  } else if (connCnt == 10) {
    return WIFI_STA_CONNECT_FAIL;
  } else {
    connCnt = connCnt < 20 ? connCnt++ : 20;
    return WIFI_STA_DISCONNECT;
  }
}

void checkWiFiConnection() {
  if (millis() - syncStaTime < WIFI_STA_SYNC_TIMER) {
    return;
  }
  syncStaTime = millis();

  uint8_t _staNum = getWiFiState();
  if (staNum == _staNum) {
    return;
  }
  staNum = _staNum;
  if (staNum == WIFI_STA_CONNECTED) {
    syncNTPTime();
    if (isWiFiRenewal) {
      Rom.setWiFi(mSSID, mPWD);
      isWiFiRenewal = false;
    }
    sendConnectResult(true);
  } else if (staNum == WIFI_STA_CONNECT_FAIL) {
    sendConnectResult(false);
  }
}

void taskWiFiClient(void* param) {
  while (1) {
    http_params_t _params;
    if (xQueueReceive(httpQueue, &_params, 1 / portTICK_RATE_MS)) {
      if (_params.type == HTTP_PING_API) {
        requsetPingApi(&_params);
        // requsetSecurityPingApi(&_params);
      } else if (_params.type == HTTP_THERMO_API) {
        requsetTemperatureApi(&_params);
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

WIFIClass::WIFIClass() {
}

void WIFIClass::begin() {
  Rom.getWiFi(&mSSID, &mPWD);
  xTaskCreate(taskWiFiClient, "WIFI_CLIENT_TASK", 1024 * 8, NULL, 3, NULL);
}


void WIFIClass::renewalData(String _data) {
  uint8_t cutIdx = _data.indexOf(',');
  mSSID = _data.substring(0, cutIdx);
  mPWD = _data.substring(cutIdx + 1, _data.length());
  WiFi.begin(mSSID.c_str(), mPWD.c_str());
  ESP_LOGE(WIFI_TAG, "Connect to %s(%s)..", mSSID, mPWD);
  staNum = WIFI_BEGIN_CONNECT;
  isWiFiRenewal = true;
  isAnswerConnect = true;
}

bool WIFIClass::isConnected() {
  return staNum == WIFI_STA_CONNECTED;
}

void WIFIClass::upload(uint8_t _type, uint8_t* _addr, uint16_t _value) {
  if (!isConnected()) {
    return;
  }
  http_params_t param = {
    .type = _type,
    .addr = _addr,
    .value = _value
  };
  xQueueSend(httpQueue, (void*)&param, 1 / portTICK_RATE_MS);
}
