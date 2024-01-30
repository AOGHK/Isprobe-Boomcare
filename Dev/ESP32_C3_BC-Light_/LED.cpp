#include "LED.h"

LEDClass LED;
Adafruit_NeoPixel pixels(1, STA_LED_PIN, NEO_GRB + NEO_KHZ800);

LEDClass::LEDClass() {
}