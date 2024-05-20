#include "Battery.h"

Battery Bat;

Battery::Battery() {
}

void Battery::init() {
  Wire.begin(BAT_SDA_PIN, BAT_SCL_PIN);
  maxlipo.begin();

  for (uint8_t cnt = 0; cnt < 4; cnt++) {
    maxlipo.cellPercent();
    delay(100);
  }
  scan();
}

void Battery::scan() {
  if (scanTime != 0 && millis() - scanTime < SCAN_BATTERY_TIMER) {
    return;
  }
  lvl = maxlipo.cellPercent();
  if (lvl > 100) {
    lvl = 100;
  }
  // ESP_LOGE("BAT", "Current Battery Level - %d %", lvl);

  if (!digitalRead(PW_STA_PIN) && lvl < LOW_BATTERY_LIMIT) {
    Light.lowBattery();
    digitalWrite(PW_CTRL_PIN, LOW);
    while (1) { delay(500); }
  }
  scanTime = millis();
}

void Battery::resetTime() {
  scanTime = 0;
}

uint8_t Battery::getLevel() {
  return lvl;
}