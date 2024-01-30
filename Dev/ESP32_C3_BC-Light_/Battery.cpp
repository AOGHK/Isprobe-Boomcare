#include "Battery.h"

Battery Bat;

Battery::Battery() {
}

void Battery::begin() {
  Wire.begin(SDA_PIN, SCL_PIN);
  isBatEnable = maxlipo.begin();

  if (!isBatEnable) {
#if DEBUG_LOG
    Serial.printf("[Battery] :: Being fail.\n");
#endif
    return;
  }

  for (uint8_t cnt = 0; cnt < 6; cnt++) {
    maxlipo.cellPercent();
    delay(50);
  }
  measure();
}

uint8_t Battery::measure() {
  if (!isBatEnable) {
    return 100;
  }

  uint8_t lvl = maxlipo.cellPercent();
  if (lvl > 100) {
    lvl = 100;
  }
#if DEBUG_LOG
  Serial.printf("[Battery] :: Current level - %d\n", lvl);
#endif
  return lvl;
}
