#ifndef _BUTTON_h
#define _BUTTON_h

#include "arduino.h"

#include "SysConf.h"
#include "PinButton.h"
#include "Rom.h"
#include "LED.h"
#include "Light.h"

class Button {
public:
  Button();
  void task();
private:
};

extern Button Btn;

#endif