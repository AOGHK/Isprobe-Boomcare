#include "Button.h"

Button Btn;

PinButton topBtn(TOP_TC_PIN, INPUT_PULLUP, LOW, 20);
PinButton botBtn(BOT_TC_PIN, INPUT_PULLUP, LOW);

bool isDimCtrl = false;

void dimmingCtrl() {
  if (!Light.isRighting()) {
    return;
  }
  isDimCtrl = !isDimCtrl;
  while (1) {
    topBtn.update();
    Light.changeBrightness(isDimCtrl);
    if (topBtn.isReleased()) {
      Rom.setBrightness(Led.getBrightness());
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
#if DEBUG_LOG
    Serial.println("[Btn] Evt -> Change theme.");
#endif
    Light.changeTheme();
  }
}

void topButtonHandle() {
  topBtn.update();
  if (topBtn.isSingleClick()) {
#if DEBUG_LOG
    Serial.println("[Btn] Evt -> Power switch.");
#endif
    Light.powerSwitch();
  }
  if (topBtn.isLongClick()) {
#if DEBUG_LOG
    Serial.println("[Btn] Evt -> Dimming ctrl.");
#endif
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
}

void Button::task() {
  xTaskCreate(taskBtnHandler, "BTN_CTRL_TASK", 1024 * 2, NULL, 3, NULL);
}