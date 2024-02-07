#ifndef _BLE_h
#define _BLE_h

#include "arduino.h"
#include "BLEDevice.h"
#include "BLEServer.h"
#include "BLEUtils.h"
#include "BLE2902.h"

#include "SysConf.h"
#include "Thermo.h"

enum {
  BLE_REMOTE_CTRL,
  BLE_SETUP_ATTR,
  BLE_REQ_ATTR,
  BLE_REQ_ADDRESS,
};

struct ble_recv_t {
  uint8_t type;
  char msg[64];
};

class BLEClass {
public:
  BLEClass();
  void begin();

private:
  void startPeripheralMode();
};

extern BLEClass BLE;
extern xQueueHandle bleQueue;

#endif
