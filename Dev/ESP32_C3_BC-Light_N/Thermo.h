#ifndef _THERMOMETER_h
#define _THERMOMETER_h

#include "arduino.h"

#include "SysConf.h"
#include "BLEDevice.h"

enum {
  THERMO_CHANGE_CONNECT = 1,
  THERMO_MEASURE_RESULT,
  THERMO_GET_SOUND_STA,
};

struct thermo_evt_t {
  uint8_t type;
  uint16_t result;
};

class Thermometer {
public:
  Thermometer();
  void task();

  bool isConnected();
  bool isSameDevice(String _address);

  uint8_t getSoundState();
  void setSoundState(uint8_t _sta);
private:
};

extern Thermometer Thermo;
extern xQueueHandle thermoQueue;

#endif