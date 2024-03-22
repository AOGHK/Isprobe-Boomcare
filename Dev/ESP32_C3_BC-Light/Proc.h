#ifndef _PROC_h
#define _PROC_h

#include "arduino.h"

#include "SysConf.h"
#include "Thermo.h"
#include "BLE.h"
#include "Light.h"
#include "LED.h"
#include "Rom.h"
#include "Wi_Fi.h"
#include "Battery.h"

class ProcClass {
public:
  ProcClass();

  void handle();
  void ping();

private:
  bool isBridgeMode = false;

  void thermoReceiver();
  void wifiReceiver();
  void bleReceiver();

  void setBridgeMode(String _cmd);
  void remoteCtrl(String _cmd);
  void userSettings(String _cmd);
  void submitAttribute(String _cmd);

  unsigned long syncPingTime = 0;
};

extern ProcClass Proc;

#endif
