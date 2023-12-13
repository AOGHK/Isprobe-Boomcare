#ifndef _BLE_h
#define _BLE_h

#include "SysEnv.h"

#include "arduino.h"
#include "BLEDevice.h"
#include "BLEServer.h"
#include "BLEUtils.h"
#include "BLE2902.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

enum {
  BLEC_SCAN_DISCOVERY = 1,
  BLEC_CONNECT_RESULT,
  BLEC_CHANGE_CONNECT,
  BLEC_CHANGE_SOUND_STA,
  BLEC_RES_TEMPERATURE,
  BLES_RECV_CTRL_DATA,
  BLES_RECV_SETUP_DATA,
  BLES_RECV_REQ_DATA,
  BLES_RECV_REQ_ADDRESS,
};

typedef struct ble_evt_data {
  uint8_t _type;
  uint16_t _num;
  String _str;
} ble_evt_t;

extern xQueueHandle blecQueue;
extern String MY_MAC_ADDRESS;

class BLE {
public:
  BLE();
  void begin();
  void setCallback(void (*evtCallback)(ble_evt_t));

  void writeData(char header, char type, String data);
  void writeData(char header, String data);
  void transferSoundState();
  void transferMyAddress();

  bool isSameThermo(String _address);
private:
  void startPeripheralMode();
};

#endif
