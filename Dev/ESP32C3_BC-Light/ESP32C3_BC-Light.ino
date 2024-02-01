#include "SysConf.h"

#include "Rom.h"
#include "BLE_Server.h"
#include "BLE_Client.h"

void setup() {
  Serial.begin(115200);
  
  Rom.begin();

  BLES.start();
  BLEC.start();
}

void loop() {
  if (Serial.available()) {
    uint8_t ctrl = Serial.readStringUntil('\n').toInt();
    BLEC.setSound(ctrl);
  }
  delay(50);
}
