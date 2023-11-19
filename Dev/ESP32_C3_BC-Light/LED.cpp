#include "LED.h"

Adafruit_NeoPixel pixels(1, STA_LED_PIN, NEO_GRB + NEO_KHZ800);

uint8_t chRed = 0;
uint8_t chGreen = 1;
uint8_t chBlue = 2;
uint8_t chBrightness = 3;

xQueueHandle ledQueue;

LED::LED() {
}

void LED::initRom() {
  EEPROM.write(1, 184);  // Power Led Brightness
  EEPROM.write(2, 0);    // Theme Num = Default 0 (Only Power LED)
  EEPROM.write(3, 255);  // RGB Theme 1
  EEPROM.write(4, 195);
  EEPROM.write(5, 0);
  EEPROM.write(6, 146);  // RGB Theme 2
  EEPROM.write(7, 208);
  EEPROM.write(8, 80);
  EEPROM.write(9, 0);  // RGB Theme 3
  EEPROM.write(10, 176);
  EEPROM.write(11, 80);
  EEPROM.write(12, 0);  // RGB Theme 4
  EEPROM.write(13, 176);
  EEPROM.write(14, 240);
  EEPROM.write(15, 112);  // RGB Theme 5
  EEPROM.write(16, 48);
  EEPROM.write(17, 160);
  for (uint16_t i = 18; i < EEPROM_SIZE; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
}

void LED::bindingData() {
  if (EEPROM.read(0) != 1) {
    EEPROM.write(0, 1);
    initRom();
  }

  brightness = EEPROM.read(1);
  themeNum = EEPROM.read(2);
  themeColors[0][0] = 0;
  themeColors[0][1] = 0;
  themeColors[0][2] = 0;
  for (int i = 3; i < 18; i += 3) {
    int pos = i / 3;
    themeColors[pos][0] = EEPROM.read(i);
    themeColors[pos][1] = EEPROM.read(i + 1);
    themeColors[pos][2] = EEPROM.read(i + 2);
  }
}

uint8_t changeRGBValue(uint8_t nColor, uint8_t tColor) {
  uint8_t color;
  if (nColor > tColor) {
    color = nColor - 1;
  } else if (nColor < tColor) {
    color = nColor + 1;
  } else {
    color = nColor;
  }
  return color;
}

void taskLedCtrl(void* param) {
  bool isChange = false;

  uint8_t tRedValue = 0;
  uint8_t tGreenValue = 0;
  uint8_t tBlueValue = 0;
  uint8_t tBrightness = 0;

  uint8_t nRedValue = 0;
  uint8_t nGreenValue = 0;
  uint8_t nBlueValue = 0;
  uint8_t nBrightness = 0;

  while (1) {
    led_evt_t evtData;
    if (xQueueReceive(ledQueue, &evtData, 10 / portTICK_RATE_MS)) {
      if (evtData._ctrl == LED_BRIGHTNESS_CTRL) {
        tBrightness = nBrightness = evtData._brightness;
        ledcWrite(chBrightness, tBrightness);
      } else {
        isChange = true;
        tRedValue = evtData._themeColors[0];
        tGreenValue = evtData._themeColors[1];
        tBlueValue = evtData._themeColors[2];
        tBrightness = evtData._brightness;
      }
    }
    if (isChange) {
      nRedValue = changeRGBValue(nRedValue, tRedValue);
      nGreenValue = changeRGBValue(nGreenValue, tGreenValue);
      nBlueValue = changeRGBValue(nBlueValue, tBlueValue);
      nBrightness = changeRGBValue(nBrightness, tBrightness);

      ledcWrite(chRed, nRedValue);
      ledcWrite(chGreen, nGreenValue);
      ledcWrite(chBlue, nBlueValue);
      ledcWrite(chBrightness, nBrightness);

      if (nRedValue == tRedValue && nGreenValue == tGreenValue
          && nBlueValue == tBlueValue && nBrightness == tBrightness) {
        isChange = false;
      }
    }
    vTaskDelay(1 / portTICK_RATE_MS);
  }
}

void LED::begin() {
  if (!EEPROM.begin(EEPROM_SIZE)) {
    while (1) {
      delay(1000);
    }
  }
  bindingData();
  ledQueue = xQueueCreate(2, sizeof(led_evt_t));

  pinMode(LED_LDO_PIN, OUTPUT);
  digitalWrite(LED_LDO_PIN, 1);

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

  pixels.begin();
  setState(false);
  
  xTaskCreatePinnedToCore(taskLedCtrl, "LED_CTRL_TASK", 1024 * 8, NULL, 1, NULL, 1);
}

void LED::setState(bool isWiFiConnected) {
  uint32_t color;
  if (isWiFiConnected) {
    color = pixels.Color(0, 0, STA_LED_BRIGHTNESS);
  } else {
    color = pixels.Color(STA_LED_BRIGHTNESS, 0, 0);
  }
  pixels.setPixelColor(0, color);
  pixels.show();
}

void LED::setTemperature(uint16_t value) {
  led_evt_t evtData = {};
  evtData._ctrl = LED_POWER_CTRL;
  if (value > 3800) {
    evtData._themeColors[0] = 119;
    evtData._themeColors[1] = 239;
    evtData._themeColors[2] = 245;
  } else if (value > 3620) {
    evtData._themeColors[0] = 175;
    evtData._themeColors[1] = 244;
    evtData._themeColors[2] = 52;
  } else {
    evtData._themeColors[0] = 245;
    evtData._themeColors[1] = 137;
    evtData._themeColors[2] = 89;
  }
  evtData._brightness = 0;
  xQueueSend(ledQueue, (void*)&evtData, 10 / portTICK_RATE_MS);
}

void LED::setThemeColor(String data) {
  themeNum = data[0] - 49;
  themeColors[themeNum][0] = data.substring(1, 4).toInt();
  themeColors[themeNum][1] = data.substring(4, 7).toInt();
  themeColors[themeNum][2] = data.substring(7, 10).toInt();

  saveThemeColor();
}

void LED::setBrightness(uint8_t data) {
  themeNum = 0;
  brightness = data;
  saveBrightness();
}

void LED::ctrlPower(bool isOn) {
  led_evt_t evtData = {};
  evtData._ctrl = LED_POWER_CTRL;
  if (isOn) {
    evtData._themeColors[0] = themeColors[themeNum][0];
    evtData._themeColors[1] = themeColors[themeNum][1];
    evtData._themeColors[2] = themeColors[themeNum][2];
    evtData._brightness = themeNum == 0 ? brightness : 0;
  } else {
    evtData._themeColors[0] = 0;
    evtData._themeColors[1] = 0;
    evtData._themeColors[2] = 0;
    evtData._brightness = 0;
  }
  xQueueSend(ledQueue, (void*)&evtData, 10 / portTICK_RATE_MS);
}

void LED::changeTheme() {
  themeNum++;
  if (themeNum > THEME_SIZE) {
    themeNum = 0;
  }
  saveThemeNumber();
  ctrlPower(true);
}

void LED::changeBrightness(bool isDim) {
  uint8_t dimValue = isDim ? brightness + 1 : brightness - 1;
  if (dimValue <= LED_MAX_BRIGHTNESS && dimValue >= LED_MIN_BRIGHTNESS) {
    brightness = dimValue;
    led_evt_t evtData = {
      ._ctrl = LED_BRIGHTNESS_CTRL,
      ._brightness = brightness
    };
    xQueueSend(ledQueue, (void*)&evtData, 10 / portTICK_RATE_MS);
  }
}

void LED::saveBrightness() {
  EEPROM.write(1, brightness);
  EEPROM.commit();
}

void LED::saveThemeNumber() {
  EEPROM.write(2, themeNum);
  EEPROM.commit();
}

void LED::saveThemeColor() {
  EEPROM.write(2, themeNum);
  uint8_t startAddr = themeNum * 3;
  EEPROM.write(startAddr, themeColors[themeNum][0]);
  EEPROM.write(startAddr + 1, themeColors[themeNum][1]);
  EEPROM.write(startAddr + 2, themeColors[themeNum][2]);
  EEPROM.commit();
}

String LED::getThemeData(char type) {
  if (type == 0x37) {
    return String(brightness);
  } else {
    uint8_t tNum = type - 49;
    char buf[9];
    sprintf(buf, "%03d%03d%03d",
            themeColors[tNum][0], themeColors[tNum][1], themeColors[tNum][2]);
    return String(buf);
  }
}