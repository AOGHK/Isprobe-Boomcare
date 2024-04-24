#ifndef _PROC_h
#define _PROC_h

#include "arduino.h"

#include "Light.h"
#include "LED.h"
#include "Rom.h"
#include "Battery.h"
#include "WIFI.h"
#include "Thermometer.h"
#include "Button.h"
#include "BLE.h"
#include "Light.h"

#define PROC_TAG "PROC"

enum {
  THERMO_DISCOVERY = 11,
  THERMO_CHANGE_CONNECT,
  THERMO_MEASURE_RESULT,
  THERMO_GET_SOUND_STA,
  WIFI_CONNECT_RESULT,
  BTN_CHANGE_POWER_STA,
  BTN_CHANGE_THEME_NUM,
  BTN_CHANGE_LED_BRIGHTNESS,
  LED_THERMO_RGB_TIMEOUT,
};

struct sta_evt_t {
  uint8_t type;
  uint16_t data;
};

class ProcClass {
public:
  ProcClass();
  void run();
  void sendEvtQueue(uint8_t _type, uint16_t _data);
  
  void ping();
  
private:
  uint32_t dotColor;
  void syncDotLed();

  bool isBridgeMode = false;
  uint16_t mTemperature;
  unsigned long syncPingTime = 0;

  void stateEventHandle();
  void bleReceiveHandle();

  void writeThermometerState();
  void writeLightState();

};

extern ProcClass Proc;

extern xQueueHandle staEventQueue;

#endif