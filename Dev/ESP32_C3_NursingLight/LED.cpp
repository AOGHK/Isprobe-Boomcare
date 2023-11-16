#include "LED.h"


Adafruit_NeoPixel pixels(1, RGB_LED_PIN, NEO_GRB + NEO_KHZ800);
uint8_t redChannel = 0;
uint8_t greenChannel = 1;
uint8_t blueChannel = 2;
uint8_t brightnessChannel = 3;

// extern xQueueHandle led_queue;

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

void led_Ctrl_Task(void* param) {
  bool isChangeLed = false;

  uint8_t tRed_Value = 0;
  uint8_t tGreen_Value = 0;
  uint8_t tBlue_Value = 0;
  uint8_t tBrightness = 0;

  uint8_t nRed_Value = 0;
  uint8_t nGreen_Value = 0;
  uint8_t nBlue_Value = 0;
  uint8_t nBrightness = 0;

  while (1) {
    led_ctrl_t lcdCtrl;
    if (xQueueReceive(led_queue, &lcdCtrl, 10 / portTICK_RATE_MS)) {
      if (lcdCtrl.typeNum == LED_POWER_ON) {
        tRed_Value = lcdCtrl.red;
        tGreen_Value = lcdCtrl.green;
        tBlue_Value = lcdCtrl.blue;
        tBrightness = lcdCtrl.brightness;
        isChangeLed = true;
      } else if (lcdCtrl.typeNum == LED_POWER_OFF) {
        tRed_Value = 0;
        tGreen_Value = 0;
        tBlue_Value = 0;
        tBrightness = 0;
        isChangeLed = true;
      } else if (lcdCtrl.typeNum == LED_BRIGHTNESS_CTRL) {
        tBrightness = nBrightness = ledBrightness;
        ledcWrite(brightnessChannel, nBrightness);
      }
#if DEBUG_LOG
      Serial.printf("LED Ctrl R(%d), G(%d), B(%d), W(%d) \n", tRed_Value, tGreen_Value, tBlue_Value, tBrightness);
#endif
    }

    if (isChangeLed) {
      nRed_Value = changeRGBValue(nRed_Value, tRed_Value);
      nGreen_Value = changeRGBValue(nGreen_Value, tGreen_Value);
      nBlue_Value = changeRGBValue(nBlue_Value, tBlue_Value);
      nBrightness = changeRGBValue(nBrightness, tBrightness);

      ledcWrite(redChannel, nRed_Value);
      ledcWrite(greenChannel, nGreen_Value);
      ledcWrite(blueChannel, nBlue_Value);
      ledcWrite(brightnessChannel, nBrightness);
      if (nRed_Value == tRed_Value && nGreen_Value == tGreen_Value
          && nBlue_Value == tBlue_Value && nBrightness == tBrightness) {
        isChangeLed = false;
        // #if DEBUG_LOG
        //         Serial.printf("Set R(%d), G(%d), B(%d), W(%d) \n", nRed_Value, nGreen_Value, nBlue_Value, nBrightness);
        // #endif
      }
    }
    vTaskDelay(1 / portTICK_RATE_MS);
  }
}

LED::LED() {
}

void LED::init() {
  pixels.begin();
  pixels.clear();
  pixels.show();

  pinMode(LED_LDO_PIN, OUTPUT);
  digitalWrite(LED_LDO_PIN, 1);

  ledcSetup(redChannel, RGB_LED_FREQ, RGB_LED_BIT);
  ledcAttachPin(RED_LED_PIN, redChannel);
  ledcWrite(redChannel, 0);

  ledcSetup(greenChannel, RGB_LED_FREQ, RGB_LED_BIT);
  ledcAttachPin(GREEN_LED_PIN, greenChannel);
  ledcWrite(greenChannel, 0);

  ledcSetup(blueChannel, RGB_LED_FREQ, RGB_LED_BIT);
  ledcAttachPin(BLUE_LED_PIN, blueChannel);
  ledcWrite(blueChannel, 0);

  ledcSetup(brightnessChannel, RGB_LED_FREQ, RGB_LED_BIT);
  ledcAttachPin(PW_LED_PIN, brightnessChannel);
  ledcWrite(brightnessChannel, 0);

  xTaskCreatePinnedToCore(led_Ctrl_Task, "LED_CTRL_TASK", 1024 * 8, NULL, 1, NULL, 1);
}

void LED::setState(uint8_t state) {
  uint32_t color;
  if (state == LED_STA_WIFI_CONNECTED) {
    color = pixels.Color(0, 0, 64);
  } else if (state == LED_STA_WIFI_DISCONNECT) {
    color = pixels.Color(64, 0, 0);
  } else {
    color = pixels.Color(0, 0, 0);
  }
  pixels.setPixelColor(0, color);
  pixels.show();
}
