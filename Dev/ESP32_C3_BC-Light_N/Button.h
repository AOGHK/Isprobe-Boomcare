#ifndef _BUTTON_h
#define _BUTTON_h

#include "arduino.h"
#include "PinButton.h"
#include "GPIO_Pin.h"
#include "Light.h"
#include "Rom.h"
#include "LED.h"

#include "Proc.h"


class Button {
public:
  Button();
  void task();
  void wakeup();
private:
};

extern Button Btn;

#endif