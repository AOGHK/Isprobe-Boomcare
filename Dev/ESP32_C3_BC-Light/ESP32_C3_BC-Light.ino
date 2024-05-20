#include "Rom.h"
#include "LED.h"
#include "Button.h"
#include "BLE.h"
#include "Battery.h"
#include "WIFI.h"
#include "Proc.h"

void setup() {
  // Serial.begin(115200);
  Led.begin();
  Btn.wakeup();
  Bat.init();

  Led.infoLog();
  Light.powerSwitch();
  delay(1000);

  BLE.begin();
  mWiFi.begin();
  Btn.task();
}

void loop() {
  Proc.run();
  Light.run();
  Bat.scan();

  Proc.ping();
  // Serial.println(ESP.getFreeHeap());
  delay(10);
}
