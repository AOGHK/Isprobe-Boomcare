#include "LED.h"
#include "Rom.h"
#include "Button.h"
#include "Light.h"
#include "BLE.h"
#include "Recv.h"

void setup() {
  Serial.begin(115200);
  Serial.println("## BC-Light (Ver 1.0) ##");

  Rom.begin();
  Led.begin();

  BLE.begin();
  Btn.task();
}

void loop() {
  if (Serial.available()) {
    int val = Serial.readStringUntil('\n').toInt();
    if (val == 2) {
      Led.nextThemeNumber();
    } else if (val == 1) {
      Led.lightOn();
    } else if (val == 0) {
      Led.lightOff();
    }
  }
  Recv.handle();
  Light.timer();
  delay(10);
}
