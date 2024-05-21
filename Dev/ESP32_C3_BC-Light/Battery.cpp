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

  lvl = getPercent();
  scanTime = millis();
  if (!digitalRead(PW_STA_PIN)) {
    checkLowLevel();
  }
}

void Battery::scan() {
  if (scanTime != 0 && millis() - scanTime < SCAN_BATTERY_TIMER) {
    return;
  }
  lvl = getPercent();
  scanTime = millis();

  if (digitalRead(PW_STA_PIN)) { return; }

  checkLowLevel();

  if (!isAlert && lvl < LOW_LEVEL_ALERT) {
    ESP_LOGE("BAT", "Low Battery Alert - %d %", lvl);
    Proc.sendEvtQueue(BAT_ALERT_LOW_LEVEL, 0);
    isAlert = true;
  } else if (lvl > LOW_LEVEL_ALERT) {
    isAlert = false;
  }
}

uint8_t Battery::getLevel() {
  return lvl;
}

uint8_t Battery::getPercent() {
  uint8_t _lvl = maxlipo.cellPercent();
  if (_lvl > 100) {
    _lvl = 100;
  }
  // ESP_LOGE("BAT", "Current Battery Level - %d %", _lvl);
  return _lvl;
}

void Battery::checkLowLevel() {
  if (lvl > LOW_LEVEL_LIMIT) {
    return;
  }
  Light.lowBattery();
  digitalWrite(PW_CTRL_PIN, LOW);
  while (1) { delay(500); }
}

void Battery::resetTime() {
  scanTime = 0;
}
