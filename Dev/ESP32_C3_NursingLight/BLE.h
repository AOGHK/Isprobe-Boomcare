#ifndef _BLE_h
#define _BLE_h

#include "arduino.h"
#include "BLEDevice.h"
#include "BLEServer.h"
#include "BLEUtils.h"
#include "BLE2902.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "SysConf.h"

class BLE {
public:
  BLE();
  void init();
  String getMacAddress();

  void setBleReceiveCallback(void (*receiveCallback)(String));

  void writeStr(char header, char type, String data);
private:
};

#endif
