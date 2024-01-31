#include "Battery.h"

Battery Bat;

Battery::Battery() {
}

void Battery::begin() {
  Wire.begin(SDA_PIN, SCL_PIN);
  isEnable = maxlipo.begin();

  if (!isEnable) {
#if DEBUG_LOG
    Serial.printf("[Battery] :: Being fail.\n");
#endif
    return;
  }

  for (uint8_t cnt = 0; cnt < 6; cnt++) {
    maxlipo.cellPercent();
    delay(50);
  }
}

uint8_t Battery::getLevel() {
  return level;
}

void Battery::scan() {
  if (!isEnable) {
    return;
  }

  if (measureTime == 0 || millis() - measureTime > SCAN_BATTERY_TIMER) {
    level = maxlipo.cellPercent();
    if (level > 100) {
      level = 100;
    }
#if DEBUG_LOG
    Serial.printf("[Battery] :: Cell percent - %d\n", level);
#endif

    if (!digitalRead(PW_STA_PIN) && level < LOW_BATTERY_LIMIT) {
#if DEBUG_LOG
      Serial.printf("[Battery] :: Low Level, Power off.\n");
#endif
      LED.lowBattery();
      digitalWrite(PW_CTRL_PIN, LOW);
      while (1) { delay(500); }
    }
    measureTime = millis();
  }
}
