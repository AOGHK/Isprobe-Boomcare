#include "MyWiFi.h"


const char* API_PING_URL = "https://192.168.219.106:3000/light/ping";
const char* API_TMP_URL = "https://192.168.219.106:3000/light/temperature";
const char* ROOT_CA =
  "-----BEGIN CERTIFICATE-----\n"
  "MIICZjCCAc+gAwIBAgIULYXxF2m6lpEctjHzYRZ0YIl56kswDQYJKoZIhvcNAQEL\n"
  "BQAwRTELMAkGA1UEBhMCQVUxEzARBgNVBAgMClNvbWUtU3RhdGUxITAfBgNVBAoM\n"
  "GEludGVybmV0IFdpZGdpdHMgUHR5IEx0ZDAeFw0yMzExMTcwNDAzNTJaFw0yMzEy\n"
  "MTcwNDAzNTJaMEUxCzAJBgNVBAYTAkFVMRMwEQYDVQQIDApTb21lLVN0YXRlMSEw\n"
  "HwYDVQQKDBhJbnRlcm5ldCBXaWRnaXRzIFB0eSBMdGQwgZ8wDQYJKoZIhvcNAQEB\n"
  "BQADgY0AMIGJAoGBANDeTTRgTBjagzbiSqk0WVTgPnwrrisQ5j9nrZd5o6XgO2Cn\n"
  "TXYsU2D2f8PyGF9QRw16W3ocrJRdWfznOXxq5rsEBue3U54yAwUa4pP9qu7ILfAv\n"
  "UsSDOxjUS9N0w9mwR+kymD/dZ0tCeFUxv7fGccc2OGdlX5KvjyZDLxlMIY1lAgMB\n"
  "AAGjUzBRMB0GA1UdDgQWBBT/IK28RZloIMPwWT0eOaWkzseWXDAfBgNVHSMEGDAW\n"
  "gBT/IK28RZloIMPwWT0eOaWkzseWXDAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3\n"
  "DQEBCwUAA4GBAAw6mCQAnEkc+SB2VwaBDBRVSktEnZVCSM40y002y7A1mWWrDjtw\n"
  "joCx17VwZ1+wVv4ZYcBFTGEy+LZ2r5WBRIdAtEtR/DaXGNBA9ZZMCBSUDTkZk5kN\n"
  "iqZYo7NGULOCqlgDbyLa2tc7LE1/xTmdueYke20iKFs7lO2IYoZ11dZ3\n"
  "-----END CERTIFICATE-----\n";

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

void MyWiFi::ping(String macAddress, uint8_t batLvl) {
  HTTPClient http;
  String params_Str = "{\"mac\":\"" + macAddress
                      + "\", \"bat_lvl\":\"" + String(batLvl) + "\"}";

  http.setConnectTimeout(1000);
  // http.setTimeout(1000);
  // Your Domain name with URL path or IP address with path
  if (http.begin(API_PING_URL)) {
    // If you need an HTTP request with a content type: application/json, use the following:
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(params_Str);
    Serial.print("(API) HTTP Response code: ");
    Serial.println(httpResponseCode);
  } else {
    Serial.println("(API) Host Connect Error..");
  }
  // Free resources
  http.end();
}

void MyWiFi::uploadTemperature(String macAddress, uint16_t tmp, String timeStr) {
  
}
