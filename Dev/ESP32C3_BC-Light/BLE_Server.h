#ifndef _BLE_SERVER_h
#define _BLE_SERVER_h

#include "arduino.h"
#include "BLEDevice.h"
#include "BLEServer.h"
#include "BLEUtils.h"
#include "BLE2902.h"

#include "SysConf.h"

class BLE_Server {
public:
  BLE_Server();
  void start();
  String getAddress();
private:
  String macAddress = "";
};

extern BLE_Server BLES;

#endif
