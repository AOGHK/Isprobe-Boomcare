#ifndef _PROC_h
#define _PROC_h

#include "arduino.h"

#include "SysEnv.h"
#include "ROM.h"
#include "BLE.h"
#include "MyWiFi.h"
#include "Battery.h"
#include "LED.h"


class ProcClass {
public:
  ProcClass();

  void begin();

private:
};

extern ProcClass Proc;

#endif