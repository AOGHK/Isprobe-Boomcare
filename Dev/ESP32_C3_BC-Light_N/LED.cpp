#include "LED.h"

LedClass Led;

Adafruit_NeoPixel pixels(1, LED_STA_PIN, NEO_GRB + NEO_KHZ800);

xQueueHandle ledCtrlQueue = xQueueCreate(2, sizeof(led_ctrl_t));

uint8_t nowRedColor = 0;
uint8_t nowGreenColor = 0;
uint8_t nowBlueColor = 0;
uint8_t nowBrightness = 0;

uint8_t targetRedColor = 0;
uint8_t targetGreenColor = 0;
uint8_t targetBlueColor = 0;
uint8_t targetBrightness = 0;

bool isDimming = false;

uint8_t setColorScale(uint8_t _nColor, uint8_t _tColor) {
  uint8_t _color;
  if (_nColor > _tColor) {
    _color = _nColor - 1;
  } else if (_nColor < _tColor) {
    _color = _nColor + 1;
  } else {
    _color = _nColor;
  }
  return _color;
}

void taskLedCtrl(void* param) {
  while (1) {
    led_ctrl_t ctrl;
    if (xQueueReceive(ledCtrlQueue, &ctrl, 0)) {
      if (ctrl.type == LED_BRIGHTNESS_CTRL) {
        nowBrightness = ctrl.brightness;
        ledcWrite(LED_PW_PIN, nowBrightness);
      } else {
        isDimming = true;
        targetRedColor = ctrl.colors.red;
        targetGreenColor = ctrl.colors.green;
        targetBlueColor = ctrl.colors.blue;
        targetBrightness = ctrl.brightness;
        ESP_LOGE(LED_TAG, "RGB Ctrl -> %d, %d, %d, %d", targetRedColor, targetGreenColor, targetBlueColor, targetBrightness);
      }
    }

    if (isDimming) {
      nowRedColor = setColorScale(nowRedColor, targetRedColor);
      nowGreenColor = setColorScale(nowGreenColor, targetGreenColor);
      nowBlueColor = setColorScale(nowBlueColor, targetBlueColor);
      nowBrightness = setColorScale(nowBrightness, targetBrightness);

      ledcWrite(LED_RED_PIN, nowRedColor);
      ledcWrite(LED_GREEN_PIN, nowGreenColor);
      ledcWrite(LED_BLUE_PIN, nowBlueColor);
      ledcWrite(LED_PW_PIN, nowBrightness);

      if (nowRedColor == targetRedColor && nowGreenColor == targetGreenColor
          && nowBlueColor == targetBlueColor && nowBrightness == targetBrightness) {
        if (ctrl.type == LED_THERMO_TIMEOUT) {
          Proc.sendEvtQueue(LED_THERMO_RGB_TIMEOUT, 0);
        }
        isDimming = false;
      }
    }
    vTaskDelay(2 / portTICK_RATE_MS);
  }
}


LedClass::LedClass() {
}

void LedClass::clear() {
  // # RGB clear
  ledcWrite(LED_PW_PIN, 0);
  ledcWrite(LED_RED_PIN, 0);
  ledcWrite(LED_GREEN_PIN, 0);
  ledcWrite(LED_BLUE_PIN, 0);
  // # Dot clear
  pixels.setPixelColor(0, 0);
  pixels.show();
}

void LedClass::infoLog() {
  ESP_LOGE(LED_TAG, "Theme Number : %d, Brightness : %d", themeNum, brightness);
  ESP_LOGE(LED_TAG, "Theme Colors : (%d, %d, %d), (%d, %d, %d), (%d, %d, %d)",
           themeColors[0].red, themeColors[0].green, themeColors[0].blue,
           themeColors[1].red, themeColors[1].green, themeColors[1].blue,
           themeColors[2].red, themeColors[2].green, themeColors[2].blue);
}

void LedClass::begin() {
  Rom.getLedAttribute(&brightness, &themeNum, themeColors);
  pixels.begin();

  ledcSetup(LED_PW_PIN, RGB_LED_FREQ, RGB_LED_BIT);
  ledcAttachPin(LED_PW_PIN, LED_PW_PIN);
  ledcWrite(LED_PW_PIN, 0);

  ledcSetup(LED_RED_PIN, RGB_LED_FREQ, RGB_LED_BIT);
  ledcAttachPin(LED_RED_PIN, LED_RED_PIN);
  ledcWrite(LED_RED_PIN, 0);

  ledcSetup(LED_GREEN_PIN, RGB_LED_FREQ, RGB_LED_BIT);
  ledcAttachPin(LED_GREEN_PIN, LED_GREEN_PIN);
  ledcWrite(LED_GREEN_PIN, 0);

  ledcSetup(LED_BLUE_PIN, RGB_LED_FREQ, RGB_LED_BIT);
  ledcAttachPin(LED_BLUE_PIN, LED_BLUE_PIN);
  ledcWrite(LED_BLUE_PIN, 0);

  clear();
  xTaskCreate(taskLedCtrl, "LED_CTRL_TASK", 1024 * 8, NULL, 2, NULL);
}

void LedClass::lightOn() {
  lightOn(LED_COLOR_CTRL);
}

void LedClass::lightOn(uint8_t _type) {
  ESP_LOGE(LED_TAG, "LED Theme Number - %d", themeNum);
  led_ctrl_t ctrl = {
    .type = _type,
    .brightness = themeNum == 0 ? brightness : 0
  };
  if (themeNum == 0) {
    ctrl.colors = { 0 },
    ctrl.brightness = brightness;
  } else {
    ctrl.colors = themeColors[themeNum - 1];
    ctrl.brightness = 0;
  }
  xQueueSend(ledCtrlQueue, (void*)&ctrl, 0);
}

void LedClass::lightOff() {
  led_ctrl_t ctrl = {
    .type = LED_COLOR_CTRL,
    .colors = { 0 },
    .brightness = 0
  };
  xQueueSend(ledCtrlQueue, (void*)&ctrl, 0);
}

uint8_t LedClass::getThemeNumber() {
  return themeNum;
}

void LedClass::changeThemeNumber() {
  themeNum++;
  if (themeNum > 3) {
    themeNum = 0;
  }
  Rom.setThemeNumber(themeNum);
  lightOn();
}

void LedClass::setThemeNumber(uint8_t _num) {
  if (_num > 3) {
    return;
  }
  themeNum = _num;
  Rom.setThemeNumber(themeNum);
  lightOn();
}


void LedClass::setThemeColor(uint8_t _num, uint8_t _red, uint8_t _green, uint8_t _blue, bool _isFixed) {
  themeNum = _num;
  Rom.setThemeNumber(themeNum);

  led_ctrl_t ctrl = {
    .type = LED_COLOR_CTRL,
    .brightness = 0,
  };
  ctrl.colors = { _red, _green, _blue };
  xQueueSend(ledCtrlQueue, (void*)&ctrl, 0);

  if (_isFixed) {
    themeColors[themeNum - 1] = ctrl.colors;
    Rom.setThemeColors(themeColors);
  }
}

led_theme_t* LedClass::getThemeColor() {
  return themeColors;
}

uint8_t LedClass::getBrightness() {
  return brightness;
}

void LedClass::setBrightness(uint8_t _brightness, bool _isFixed) {
  themeNum = 0;
  Rom.setThemeNumber(themeNum);

  led_ctrl_t ctrl = {
    .type = LED_COLOR_CTRL,
  };

  ctrl.colors = { 0 };
  if (_isFixed) {
    brightness = _brightness;
    ctrl.brightness = brightness;
    Rom.setBrightness(brightness);
  } else {
    ctrl.brightness = _brightness;
  }
  xQueueSend(ledCtrlQueue, (void*)&ctrl, 0);
}

void LedClass::reducesBrightness() {
  if (themeNum != 0 || brightness == LED_MIN_BRIGHTNESS) {
    return;
  }
  uint16_t _brightness = brightness - 3;
  if (_brightness < LED_MIN_BRIGHTNESS) {
    brightness = LED_MIN_BRIGHTNESS;
  } else {
    brightness = _brightness;
  }
  led_ctrl_t ctrl = {
    .type = LED_BRIGHTNESS_CTRL,
    .brightness = brightness
  };
  xQueueSend(ledCtrlQueue, (void*)&ctrl, 0);
}

void LedClass::increasesBrightness() {
  if (themeNum != 0 || brightness == LED_MAX_BRIGHTNESS) {
    return;
  }
  uint16_t _brightness = brightness + 3;
  if (_brightness > LED_MAX_BRIGHTNESS) {
    brightness = LED_MAX_BRIGHTNESS;
  } else {
    brightness = _brightness;
  }
  led_ctrl_t ctrl = {
    .type = LED_BRIGHTNESS_CTRL,
    .brightness = brightness
  };
  xQueueSend(ledCtrlQueue, (void*)&ctrl, 0);
}

void LedClass::setThermoColor(uint16_t _thermo) {
  led_ctrl_t ctrl = {
    .type = LED_COLOR_CTRL,
    .brightness = 0,
  };
  if (_thermo > 3860) {
    ctrl.colors.red = 255;
    ctrl.colors.green = 0;
    ctrl.colors.blue = 0;
  } else if (_thermo > 3750) {
    ctrl.colors.red = 255;
    ctrl.colors.green = 55;
    ctrl.colors.blue = 0;
  } else if (_thermo > 3570) {
    ctrl.colors.red = 0;
    ctrl.colors.green = 255;
    ctrl.colors.blue = 0;
  } else {
    ctrl.colors.red = 0;
    ctrl.colors.green = 0;
    ctrl.colors.blue = 255;
  }
  xQueueSend(ledCtrlQueue, (void*)&ctrl, 0);
}

void LedClass::setDot(uint32_t _color) {
  pixels.setPixelColor(0, _color);
  pixels.show();
}

void LedClass::setLedColor(uint8_t _red, uint8_t _green, uint8_t _blue) {
  ledcWrite(LED_RED_PIN, _red);
  ledcWrite(LED_GREEN_PIN, _green);
  ledcWrite(LED_BLUE_PIN, _blue);
}
