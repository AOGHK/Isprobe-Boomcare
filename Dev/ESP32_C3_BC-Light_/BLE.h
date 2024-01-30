#ifndef _BLE_h
#define _BLE_h

#include "arduino.h"
#include "BLEDevice.h"
#include "BLEServer.h"
#include "BLEUtils.h"
#include "BLE2902.h"

#include "SysEnv.h"

struct ble_evt_t {
  uint8_t _type;
  uint16_t _num;
  String _str;
};

class BLEClass {
public:
  BLEClass();
  void begin();
  String getMyAddress();

  bool isSameBoomcare(String _address);

private:
  String myMacAddress = "";
  void startPeripheralMode();
};

extern xQueueHandle blecQueue;
extern BLEClass BLE;

#endif