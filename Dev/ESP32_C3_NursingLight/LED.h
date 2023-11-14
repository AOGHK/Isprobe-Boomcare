#ifndef _LED_h
#define _LED_h

#include "arduino.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "Adafruit_NeoPixel.h"

#include "SysConf.h"

class LED {
public:
  LED();
  void init();
  void setState(uint8_t state);
private:
};

#endif