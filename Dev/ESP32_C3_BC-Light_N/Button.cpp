#include "Button.h"

Button Btn;
PinButton topBtn(TOP_TC_PIN, INPUT_PULLUP, LOW, 20);
PinButton botBtn(BOT_TC_PIN, INPUT_PULLUP, LOW);
bool isDimCtrl = false;

void dimmingCtrl() {
  if (!Light.isActivated()) {
    return;
  }
  isDimCtrl = !isDimCtrl;
  while (1) {
    topBtn.update();
    Light.changeBrightness(isDimCtrl);
    if (topBtn.isReleased()) {
      Rom.setBrightness(Led.getBrightness());
      Proc.sendEvtQueue(BTN_CHANGE_LED_BRIGHTNESS, 0);
      break;
    }
    vTaskDelay(30 / portTICK_RATE_MS);
  }
}

void powerButtonHandle() {
  if (digitalRead(PW_STA_PIN) || !digitalRead(PW_BTN_PIN)) {
    return;
  }

  uint8_t pressCnt = 0;
  while (digitalRead(PW_BTN_PIN)) {
    pressCnt++;
    if (pressCnt == 5) {
      Led.clear();
    }
    vTaskDelay(50 / portTICK_RATE_MS);
  }

  if (pressCnt >= 5) {
    digitalWrite(PW_CTRL_PIN, LOW);
  }
}

void bottomButtonHandle() {
  botBtn.update();
  if (botBtn.isDoubleClick()) {
    Light.changeTheme();
    Proc.sendEvtQueue(BTN_CHANGE_THEME_NUM, 0);
  }
}

void topButtonHandle() {
  topBtn.update();
  if (topBtn.isSingleClick()) {
    Light.powerSwitch();
    Proc.sendEvtQueue(BTN_CHANGE_POWER_STA, 0);
  }
  if (topBtn.isLongClick() && Led.getThemeNumber() == 0) {
    dimmingCtrl();
  }
}

void taskBtnHandler(void* param) {
  while (1) {
    powerButtonHandle();
    topButtonHandle();
    bottomButtonHandle();
    vTaskDelay(10 / portTICK_RATE_MS);
  }
}

Button::Button() {
  pinMode(PW_BTN_PIN, INPUT_PULLDOWN);
  pinMode(PW_STA_PIN, INPUT);  // INPUT_PULLUP
  pinMode(PW_BTN_PIN, INPUT);
  pinMode(PW_CTRL_PIN, OUTPUT);
  digitalWrite(PW_CTRL_PIN, HIGH);
}

void Button::task() {
  xTaskCreate(taskBtnHandler, "BTN_CTRL_TASK", 1024 * 2, NULL, 3, NULL);
}

void Button::wakeup() {
  Led.setDot(DOT_RED_COLOR);
  while (digitalRead(PW_BTN_PIN)) { delay(10); }
}