#include "LED.h"

LEDClass LED;
Adafruit_NeoPixel pixels(1, STA_LED_PIN, NEO_GRB + NEO_KHZ800);

xQueueHandle ledQueue = xQueueCreate(2, sizeof(led_evt_t));

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
}