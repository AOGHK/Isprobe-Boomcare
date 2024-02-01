#ifndef _BLE_CLIENT_h
#define _BLE_CLIENT_h

#include "arduino.h"
#include "BLEDevice.h"

#include "SysConf.h"

class BLE_Client {
public:
  BLE_Client();
  void start();

  void setSound(bool _enable);
private:
};

extern BLE_Client BLEC;

#endif
