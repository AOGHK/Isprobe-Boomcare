#include "LED.h"

LEDClass LED;
Adafruit_NeoPixel pixels(1, STA_LED_PIN, NEO_GRB + NEO_KHZ800);

xQueueHandle ledQueue = xQueueCreate(2, sizeof(led_evt_t));

uint8_t channel_red = 0;
uint8_t channel_green = 1;
uint8_t channel_blue = 2;
uint8_t channel_brightness = 3;

uint8_t nRedValue = 0;
uint8_t nGreenValue = 0;
uint8_t nBlueValue = 0;
uint8_t nBrightness = 0;

uint8_t changeColorValue(uint8_t nowColor, uint8_t targetColor) {
  uint8_t color;
  if (nowColor > targetColor) {
    color = nowColor - CTRL_STEP_SIZE;
  } else if (nowColor < targetColor) {
    color = nowColor + CTRL_STEP_SIZE;
  } else {
    color = nowColor;
  }
  return color;
}

void taskLedCtrl(void* param) {
  bool isChange = false;
  uint8_t tRedValue = 0;
  uint8_t tGreenValue = 0;
  uint8_t tBlueValue = 0;
  uint8_t tBrightness = 0;

  while (1) {
    led_evt_t evtData;
    if (xQueueReceive(ledQueue, &evtData, 10 / portTICK_RATE_MS)) {
      if (evtData._ctrl == LED_BRIGHTNESS_CTRL) {
        tBrightness = nBrightness = evtData._brightness;
        ledcWrite(channel_brightness, tBrightness);
      } else {
        isChange = true;
        tRedValue = evtData._themeColors[0];
        tGreenValue = evtData._themeColors[1];
        tBlueValue = evtData._themeColors[2];
        tBrightness = evtData._brightness;
#if DEBUG_LOG
        Serial.printf("[LED] :: Light Ctrl - R(%d), G(%d), B(%d), W(%d)\n", tRedValue, tGreenValue, tBlueValue, tBrightness);
#endif
      }
    }

    if (isChange) {
      nRedValue = changeColorValue(nRedValue, tRedValue);
      nGreenValue = changeColorValue(nGreenValue, tGreenValue);
      nBlueValue = changeColorValue(nBlueValue, tBlueValue);
      nBrightness = changeColorValue(nBrightness, tBrightness);

      ledcWrite(channel_red, nRedValue);
      ledcWrite(channel_green, nGreenValue);
      ledcWrite(channel_blue, nBlueValue);
      ledcWrite(channel_brightness, nBrightness);

      if (nRedValue == tRedValue && nGreenValue == tGreenValue
          && nBlueValue == tBlueValue && nBrightness == tBrightness) {
        isChange = false;
      }
    }
    vTaskDelay(1 / portTICK_RATE_MS);
  }
}


LEDClass::LEDClass() {
}

void LEDClass::begin() {
  uint8_t brightness;
  uint8_t themeNum = 0;
  uint8_t themeColors[THEME_SIZE + 1][3];
  ROM.getLedAttribute(&brightness, &themeNum, themeColors);

#if DEBUG_LOG
  Serial.printf("[LED] :: Theme Num - %d, Brightness - %d.\n", brightness, themeNum);
  for (uint8_t _size = 0; _size < 6; _size++) {
    Serial.printf("[LED] :: Theme index - %d, Color - %d, %d,%d\n", _size,
                  themeColors[_size][0], themeColors[_size][1], themeColors[_size][2]);
  }
#endif

  ledcSetup(channel_red, RGB_LED_FREQ, RGB_LED_BIT);
  ledcAttachPin(RED_LED_PIN, channel_red);
  ledcWrite(channel_red, 0);

  ledcSetup(channel_green, RGB_LED_FREQ, RGB_LED_BIT);
  ledcAttachPin(GREEN_LED_PIN, channel_green);
  ledcWrite(channel_green, 0);

  ledcSetup(channel_blue, RGB_LED_FREQ, RGB_LED_BIT);
  ledcAttachPin(BLUE_LED_PIN, channel_blue);
  ledcWrite(channel_blue, 0);

  ledcSetup(channel_brightness, RGB_LED_FREQ, RGB_LED_BIT);
  ledcAttachPin(PW_LED_PIN, channel_brightness);
  ledcWrite(channel_brightness, 0);

  pixels.begin();

  xTaskCreate(taskLedCtrl, "LED_CTRL_TASK", 1024 * 8, NULL, 2, NULL);
}


uint8_t LEDClass::getThemeNumber() {
  return themeNum;
}

String LEDClass::getThemeColor(char _num) {
  uint8_t tNum = _num - 49;
  char buf[9];
  sprintf(buf, "%03d%03d%03d",
          themeColors[tNum][0], themeColors[tNum][1], themeColors[tNum][2]);
  return String(buf);
}

void LEDClass::setThemeColor(String data) {
  themeNum = data[0] - 49;
  themeColors[themeNum][0] = data.substring(1, 4).toInt();
  themeColors[themeNum][1] = data.substring(4, 7).toInt();
  themeColors[themeNum][2] = data.substring(7, 10).toInt();
  ROM.setThemeColor(themeNum, themeColors[themeNum][0], themeColors[themeNum][1], themeColors[themeNum][2]);
}


String LEDClass::getBrightness() {
  return String(brightness);
}

void LEDClass::setBrightness(uint8_t _brightness) {
}

void LEDClass::setDotColor(uint8_t _sta) {
  uint32_t staColor;
  if (_sta == LED_STA_CHARGE) {
    staColor = pixels.Color(0, STA_LED_BRIGHTNESS, 0);
  } else if (_sta == LED_STA_WIFI_CONN) {
    staColor = pixels.Color(0, 0, STA_LED_BRIGHTNESS);
  } else {
    staColor = pixels.Color(STA_LED_BRIGHTNESS, 0, 0);
  }
  pixels.setPixelColor(0, staColor);
  pixels.show();
}