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

#define BOOMCARE_SCAN_SEC 5
#define BLEC_SCAN_DELAY 100
#define BLEC_AD_DELAY 500
#define BLES_AD_DELAY 100

enum {
  BLE_CHANGE_CONNECT = 1,
  BLE_MEASURE_TEMPERATURE,
  BLE_RECV_CTRL_DATA,
  BLE_RECV_SETUP_DATA,
  BLE_RECV_REQ_DATA,
  BLE_RECV_REQ_ADDRESS,
};

typedef struct ble_evt_data {
  uint8_t _type;
  uint16_t _num;
  String _str;
} ble_evt_t;

class BLE {
public:
  BLE();
  void begin();
  String getMacAddress();
  void writeData(char type, String data);

  void setEvnetCallback(void (*evtCallback)(ble_evt_t));
private:
  String deviceMacAddress = "";
  BLECharacteristic* sCharacteristic = NULL;

  void startServer();
};

#endif
