#include "Proc.h"

ProcClass Proc;

ProcClass::ProcClass() {
  pinMode(PW_STA_PIN, INPUT_PULLUP);
  pinMode(PW_BTN_PIN, INPUT_PULLDOWN);
  pinMode(PW_CTRL_PIN, OUTPUT);
}

void ProcClass::begin() {
  digitalWrite(PW_CTRL_PIN, HIGH);
  LED.begin();
  LED.setDotColor();
  while (digitalRead(PW_BTN_PIN)) { delay(10); }
}