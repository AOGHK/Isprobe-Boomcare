#ifndef _BLE_h
#define _BLE_h

#include "arduino.h"
#include "BLEDevice.h"
#include "BLEServer.h"
#include "BLEUtils.h"
#include "BLE2902.h"

#include "Thermometer.h"

struct ble_recv_t {
  uint8_t header;
  uint8_t* cmd;
  uint8_t len;
};

class BLEClass {
public:
  BLEClass();
  void begin();
  uint8_t* getAddress();

  void write(uint8_t _header, uint8_t _value);
  void write(uint8_t* _data, uint8_t _len);

private:
  uint8_t macAddress[6] = { 0 };
  void startPeripheralMode();
};

extern BLEClass BLE;
extern xQueueHandle bleQueue;

#endif
