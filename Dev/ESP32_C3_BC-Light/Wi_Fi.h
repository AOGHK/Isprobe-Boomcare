#ifndef _WI_FI_h
#define _WI_FI_h

#include "arduino.h"
#include "WiFi.h"
#include "HTTPClient.h"
#include "time.h"

#include "SysConf.h"
#include "Rom.h"

typedef enum {
  WIFI_BEGIN_CONNECT = 1,
  WIFI_STA_CONNECTED,
  WIFI_STA_CONNECT_FAIL,
  WIFI_STA_DISCONNECT,
} wifi_sta_t;

typedef enum {
  HTTP_PING_API = 1,
  HTTP_THERMO_API,
} http_api_t;

struct http_params_t {
  uint8_t type;
  char macAddress[18];
  uint8_t batLevel;
  thermo_data_t thermo;
};

class Wi_Fi {
public:
  Wi_Fi();
  void begin();
  bool isConnected();
  void renewalData(String _data);

  void syncPing(String _addr, uint8_t _batLevel);
  void uploadThermo(String _addr, uint16_t _value);

private:
};

extern Wi_Fi mWiFi;
extern xQueueHandle wifiConnQueue;
extern xQueueHandle httpQueue;

#endif