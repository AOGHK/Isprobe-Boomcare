#ifndef _THERMOMETER_h
#define _THERMOMETER_h

#include "arduino.h"

#include "BLEDevice.h"
#include "Proc.h"

class Thermometer {
public:
  Thermometer();
  void task();

  bool isConnected();
  uint8_t* getAddress();

  uint8_t getSoundState();
  void setSoundState(uint8_t _sta);
private:
};

extern Thermometer Thermo;

#endif