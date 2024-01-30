#ifndef _MYWIFI_h
#define _MYWIFI_h

#include "arduino.h"
#include "WiFi.h"
#include "HTTPClient.h"
#include "time.h"

#include "SysEnv.h"
#include "ROM.h"

enum {
  WIFI_STA_REQ_CONNECT = 1,
  WIFI_STA_CONNECTING,
  WIFI_STA_CONNECTED,
};

typedef struct tmp_param_t {
  uint8_t tmp[2];
  uint8_t time[6];
};

class MyWiFi {
public:
  MyWiFi();
  void begin();
private:
};

extern MyWiFi mWiFi;

#endif