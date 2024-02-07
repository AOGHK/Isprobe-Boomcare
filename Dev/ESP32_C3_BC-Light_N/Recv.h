#ifndef _RECV_h
#define _RECV_h

#include "arduino.h"

#include "SysConf.h"
#include "Thermo.h"
#include "BLE.h"
#include "Light.h"
#include "LED.h"
#include "Rom.h"

class RecvClass {
public:
  RecvClass();

  void handle();

private:
  void thermoReceiver();
  void bleReceiver();
};

extern RecvClass Recv;

#endif
