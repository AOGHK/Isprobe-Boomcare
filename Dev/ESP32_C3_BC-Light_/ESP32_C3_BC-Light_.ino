#include "SysEnv.h"
#include "ROM.h"
#include "BLE.h"
#include "MyWiFi.h"
#include "Battery.h"
#include "LED.h"

void setup() {
  Serial.begin(115200);

  ROM.begin();
  BLE.begin();
  LED.begin();
}

void loop() {
  // put your main code here, to run repeatedly:

}
