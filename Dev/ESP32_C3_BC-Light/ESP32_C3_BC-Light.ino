#include "SysConf.h"
#include "LED.h"
#include "Rom.h"
#include "Button.h"
#include "Light.h"
#include "BLE.h"
#include "Proc.h"
#include "Battery.h"
#include "Wi_Fi.h"

void syncDot() {
  uint32_t dotColor = digitalRead(PW_STA_PIN) ? DOT_GREEN_COLOR : (mWiFi.isConnected() ? DOT_BLUE_COLOR : DOT_RED_COLOR);
  Led.setDot(dotColor);
}

void waitForActive() {
  syncDot();
  while (digitalRead(PW_BTN_PIN)) { delay(10); }
}


void setup() {
  Serial.begin(115200);
  // Serial.println("###### BC-Light (Ver 1.0) ######");

  pinMode(PW_STA_PIN, INPUT_PULLUP);
  pinMode(PW_BTN_PIN, INPUT_PULLDOWN);
  pinMode(PW_CTRL_PIN, OUTPUT);
  digitalWrite(PW_CTRL_PIN, HIGH);

  Rom.begin();
  Led.begin();

  waitForActive();

  Bat.init();
  Bat.scan();

  Light.powerSwitch();

  BLE.begin();
  mWiFi.begin();
  Btn.task();
}

void loop() {
  if (Serial.available()) {
    int cmd = Serial.readStringUntil('\n').toInt();
    if (cmd == 0) {
      Rom.clear();
    }
  }
  
  Bat.scan();
  Proc.handle();
  Light.timer();
  Proc.ping();

  syncDot();
  delay(10);
}
