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
  BLEC_CHANGE_CONNECT,
  BLEC_CHANGE_SOUND_MODE,
  BLEC_RES_TEMPERATURE,
  BLEC_RES_STA_SOUND,
  BLEC_CONNECT_TIMEOUT,
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

extern xQueueHandle bcQueue;

class BLE {
public:
  BLE();
  void begin();
  String getMacAddress();
  void writeData(char header, char type, String data);
  void writeData(char header, String data);
  String getBoomcareAddress();

  void setEvnetCallback(void (*evtCallback)(ble_evt_t));
private:
  String deviceMacAddress = "";
  BLECharacteristic* sCharacteristic = NULL;

  void startServer();
};

#endif
