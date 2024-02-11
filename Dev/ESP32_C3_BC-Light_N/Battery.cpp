#include "Battery.h"

Battery Bat;

Battery::Battery() {
}

void Battery::init() {
  Wire.begin(SDA_PIN, SCL_PIN);
  maxlipo.begin();

  for (uint8_t cnt = 0; cnt < 4; cnt++) {
    maxlipo.cellPercent();
    delay(100);
  }
}

void Battery::scan() {
  if (scanTime != 0 && millis() - scanTime < SCAN_BATTERY_TIMER) {
    return;
  }
  lvl = maxlipo.cellPercent();
  if (lvl > 100) {
    lvl = 100;
  }
// #if DEBUG_LOG
//   Serial.printf("[Battery] Current percent : %d.\n", lvl);
// #endif
  checkLowLevel();
  scanTime = millis();
}

void Battery::checkLowLevel() {
  if (!digitalRead(PW_STA_PIN) && lvl < LOW_BATTERY_LIMIT) {
    for (uint8_t cnt = 0; cnt < 6; cnt++) {
      if (cnt % 2 == 0) {
        Led.setRGBColor(255, 255, 255);
      } else {
        Led.setRGBColor(0, 0, 0);
      }
      delay(200);
    }
    digitalWrite(PW_CTRL_PIN, LOW);
    while (1) { delay(500); }
  }
}

uint8_t Battery::getLevel() {
  return lvl;
}
