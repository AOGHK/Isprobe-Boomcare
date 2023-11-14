#ifndef _MYWIFI_h
#define _MYWIFI_h

#include "arduino.h"
#include "WiFi.h"
#include "HTTPClient.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "SysConf.h"

class MyWiFi {
public:
  MyWiFi();
  void sync();

private:
  void checkConnected();
};

#endif
