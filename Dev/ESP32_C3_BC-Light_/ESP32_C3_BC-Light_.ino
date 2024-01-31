#include "SysEnv.h"
#include "ROM.h"
#include "BLE.h"
#include "MyWiFi.h"
#include "Battery.h"
#include "Proc.h"

void setup() {
  Serial.begin(115200);
  Proc.begin();

  Bat.scan();

  ROM.begin();
  BLE.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
}
