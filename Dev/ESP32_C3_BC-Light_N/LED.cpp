#include "LED.h"

LedClass Led;
xQueueHandle ledQueue = xQueueCreate(2, sizeof(led_ctrl_t));

Adafruit_NeoPixel pixels(1, STA_LED_PIN, NEO_GRB + NEO_KHZ800);

uint8_t chRed = 0;
uint8_t chGreen = 1;
uint8_t chBlue = 2;
uint8_t chBrightness = 3;

uint8_t nowRedLvl = 0;
uint8_t nowGreenLvl = 0;
uint8_t nowBlueLvl = 0;
uint8_t nowBrightness = 0;

uint8_t targetRedLvl = 0;
uint8_t targetGreenLvl = 0;
uint8_t targetBlueLvl = 0;
uint8_t targetBrightness = 0;

bool isDimming = false;

uint8_t setScaleLevel(uint8_t nowLvl, uint8_t targetLvl) {
  uint8_t lvl;
  if (nowLvl > targetLvl) {
    lvl = nowLvl - LED_CTRL_STEP_SIZE;
  } else if (nowLvl < targetLvl) {
    lvl = nowLvl + LED_CTRL_STEP_SIZE;
  } else {
    lvl = nowLvl;
  }
  return lvl;
}

void taskLedCtrl(void* param) {
  while (1) {
    led_ctrl_t ctrl;
    if (xQueueReceive(ledQueue, &ctrl, 0)) {
      if (ctrl.type == LED_BRIGHTNESS_CTRL) {
        targetBrightness = nowBrightness = ctrl.brightness;
        ledcWrite(chBrightness, targetBrightness);
      } else {
        isDimming = true;
        targetRedLvl = ctrl.colors[0];
        targetGreenLvl = ctrl.colors[1];
        targetBlueLvl = ctrl.colors[2];
        targetBrightness = ctrl.brightness;
#if DEBUG_LOG
        Serial.printf("[LED] :: Ctrl - R(%d), G(%d), B(%d), W(%d)\n", targetRedLvl, targetGreenLvl, targetBlueLvl, targetBrightness);
#endif
      }
    }

    if (isDimming) {
      nowRedLvl = setScaleLevel(nowRedLvl, targetRedLvl);
      nowGreenLvl = setScaleLevel(nowGreenLvl, targetGreenLvl);
      nowBlueLvl = setScaleLevel(nowBlueLvl, targetBlueLvl);
      nowBrightness = setScaleLevel(nowBrightness, targetBrightness);
      ledcWrite(chRed, nowRedLvl);
      ledcWrite(chGreen, nowGreenLvl);
      ledcWrite(chBlue, nowBlueLvl);
      ledcWrite(chBrightness, nowBrightness);
      if (nowRedLvl == targetRedLvl && nowGreenLvl == targetGreenLvl
          && nowBlueLvl == targetBlueLvl && nowBrightness == targetBrightness) {
        isDimming = false;
      }
    }
    vTaskDelay(2 / portTICK_RATE_MS);
  }
}

LedClass::LedClass() {
}

void LedClass::begin() {
  Rom.getLedAttribute(&brightness, &themeNum, themeColors);
#if DEBUG_LOG
  Serial.printf("[LED] :: Attr - Brightness(%d), ThemeNum(%d)\n", brightness, themeNum);
  Serial.print("[LED] :: Colors -> ");
  for (uint8_t idx = 0; idx <= LED_THEME_SIZE; idx++) {
    Serial.printf("(%d : %d, %d, %d) | ", idx, themeColors[idx][0], themeColors[idx][1], themeColors[idx][2]);
  }
  Serial.println();
#endif
  pixels.begin();

  ledcSetup(chRed, RGB_LED_FREQ, RGB_LED_BIT);
  ledcAttachPin(RED_LED_PIN, chRed);
  ledcWrite(chRed, 0);
  ledcSetup(chGreen, RGB_LED_FREQ, RGB_LED_BIT);
  ledcAttachPin(GREEN_LED_PIN, chGreen);
  ledcWrite(chGreen, 0);
  ledcSetup(chBlue, RGB_LED_FREQ, RGB_LED_BIT);
  ledcAttachPin(BLUE_LED_PIN, chBlue);
  ledcWrite(chBlue, 0);
  ledcSetup(chBrightness, RGB_LED_FREQ, RGB_LED_BIT);
  ledcAttachPin(PW_LED_PIN, chBrightness);
  ledcWrite(chBrightness, 0);

  clear();
  xTaskCreate(taskLedCtrl, "LED_CTRL_TASK", 1024 * 8, NULL, 2, NULL);
}

void LedClass::clear() {
  // # RGB clear
  ledcWrite(chRed, 0);
  ledcWrite(chGreen, 0);
  ledcWrite(chBlue, 0);
  ledcWrite(chBrightness, 0);
  // # Dot clear
  pixels.setPixelColor(0, 0);
  pixels.show();
}

void LedClass::setDot(uint32_t _color) {
  if (dotColor == _color) {
    return;
  }

  dotColor = _color;
  pixels.setPixelColor(0, dotColor);
  pixels.show();
}

void LedClass::lightOn() {
  led_ctrl_t ctrl = {
    .type = LED_POWER_CTRL,
    .colors = { themeColors[themeNum][0], themeColors[themeNum][1], themeColors[themeNum][2] },
    .brightness = themeNum == 0 ? brightness : 0
  };
  xQueueSend(ledQueue, (void*)&ctrl, 0);
}

void LedClass::lightOff() {
  led_ctrl_t ctrl = {
    .type = LED_POWER_CTRL,
    .colors = { 0, 0, 0 },
    .brightness = 0
  };
  xQueueSend(ledQueue, (void*)&ctrl, 0);
}

uint8_t LedClass::getThemeNumber() {
  return themeNum;
}

void LedClass::nextThemeNumber() {
  themeNum++;
  if (themeNum > LED_THEME_SIZE) {
    themeNum = 0;
  }
  Rom.setThemeNumber(themeNum);
  lightOn();
}

void LedClass::setThemeNumber(uint8_t _num) {
  if (_num > LED_THEME_SIZE) {
    return;
  }
  themeNum = _num;
  Rom.setThemeNumber(themeNum);
  lightOn();
}

void LedClass::setThemeColor(String data) {
  if (data.length() != 11) {
    return;
  }
  
  uint8_t _red = data.substring(2, 5).toInt();
  uint8_t _green = data.substring(5, 8).toInt();
  uint8_t _blue = data.substring(8, 11).toInt();
  led_ctrl_t ctrl = {
    .type = LED_POWER_CTRL,
    .colors = { _red, _green, _blue },
    .brightness = 0
  };
  xQueueSend(ledQueue, (void*)&ctrl, 0);

  if (data[1] == 0x31) {
    themeNum = data[0] - 49;
    themeColors[themeNum][0] = _red;
    themeColors[themeNum][1] = _green;
    themeColors[themeNum][2] = _blue;
    Rom.setThemeColor(themeNum, _red, _green, _blue);
  }
}

String LedClass::getThemeColor(uint8_t _num) {
  char buf[9];
  sprintf(buf, "%03d%03d%03d",
          themeColors[_num][0], themeColors[_num][1], themeColors[_num][2]);
  return String(buf);
}

uint8_t LedClass::getBrightness() {
  return brightness;
}

void LedClass::reducesBrightness() {
  if (themeNum != 0) {
    return;
  }

  uint16_t _brightness = brightness - LED_CTRL_STEP_SIZE;
  if (_brightness < LED_MIN_BRIGHTNESS) {
    return;
  }

  brightness = _brightness;
  led_ctrl_t ctrl = {
    .type = LED_BRIGHTNESS_CTRL,
    .brightness = brightness
  };
  xQueueSend(ledQueue, (void*)&ctrl, 0);
}

void LedClass::increasesBrightness() {
  if (themeNum != 0) {
    return;
  }

  uint16_t _brightness = brightness + LED_CTRL_STEP_SIZE;
  if (_brightness > LED_MAX_BRIGHTNESS) {
    return;
  }

  brightness = _brightness;
  led_ctrl_t ctrl = {
    .type = LED_BRIGHTNESS_CTRL,
    .brightness = brightness
  };
  xQueueSend(ledQueue, (void*)&ctrl, 0);
}

void LedClass::setBrightness(uint8_t _brightness, bool _fixed) {
  led_ctrl_t ctrl = {
    .type = LED_BRIGHTNESS_CTRL,
    .brightness = _brightness
  };
  xQueueSend(ledQueue, (void*)&ctrl, 0);
  if (_fixed) {
    brightness = _brightness;
    Rom.setBrightness(brightness);
  }
}

void LedClass::setThermoColor(uint16_t _thermo) {
  led_ctrl_t ctrl = {
    .type = LED_POWER_CTRL,
    .brightness = 0,
  };
  if (_thermo > 3860) {
    ctrl.colors[0] = 255;
    ctrl.colors[1] = 0;
    ctrl.colors[2] = 0;
  } else if (_thermo > 3750) {
    ctrl.colors[0] = 255;
    ctrl.colors[1] = 55;
    ctrl.colors[2] = 0;
  } else if (_thermo > 3570) {
    ctrl.colors[0] = 0;
    ctrl.colors[1] = 255;
    ctrl.colors[2] = 0;
  } else {
    ctrl.colors[0] = 0;
    ctrl.colors[1] = 0;
    ctrl.colors[2] = 255;
  }
  xQueueSend(ledQueue, (void*)&ctrl, 0);
}

void LedClass::setRGBColor(uint8_t _red, uint8_t _green, uint8_t _blue) {
  ledcWrite(chRed, _red);
  ledcWrite(chGreen, _green);
  ledcWrite(chBlue, _blue);
}