#ifndef _WI_FI_h
#define _WI_FI_h

#include "arduino.h"
#include "WiFi.h"
#include "HTTPClient.h"
#include "time.h"
#include "Rom.h"
#include <WiFiClientSecure.h>

#include "Proc.h"

#define WIFI_STA_SYNC_TIMER 1000
#define WIFI_REQ_PING_TIMER (1000 * 60 * 10)

#define WIFI_TAG "WIFI"

enum {
  WIFI_BEGIN_CONNECT = 1,
  WIFI_STA_CONNECTED,
  WIFI_STA_CONNECT_FAIL,
  WIFI_STA_DISCONNECT,
  HTTP_PING_API,
  HTTP_THERMO_API,
};

struct http_params_t {
  uint8_t type;
  uint8_t* addr;
  uint16_t value;
};

class WIFIClass {
public:
  WIFIClass();
  void begin();

  void renewalData(String _data);
  bool isConnected();

  void upload(uint8_t _type, uint8_t* _addr, uint16_t _value);

private:
};

extern WIFIClass mWiFi;
extern xQueueHandle httpQueue;

#endif