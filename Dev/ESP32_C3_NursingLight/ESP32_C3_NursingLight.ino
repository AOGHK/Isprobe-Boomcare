#include "SysConf.h"
#include "BLE.h"
#include "LED.h"

SysConf sysConf;
BLE ble;
LED led;

bool isDimCtrl = true;
bool isLightingOn = false;
uint8_t lightCtrlNum = 0;

unsigned long thermoLightTime = 0;
unsigned long userLightTime = 0;
unsigned long userLightRuntime = 0;

void lightOn(uint8_t ctrlNum) {
  isLightingOn = true;
  lightCtrlNum = ctrlNum;
  thermoLightTime = 0;
  sysConf.transferLEDEvent(LED_POWER_ON);
}

void lightOff() {
  isLightingOn = false;
  thermoLightTime = 0;
  userLightTime = 0;
  sysConf.transferLEDEvent(LED_POWER_OFF);
}

void userLightTimer() {
  if (userLightTime == 0) {
    return;
  }
  if (millis() - userLightTime > userLightRuntime) {
    lightOff();
  }
}

void thermoLightTimer() {
  if (thermoLightTime == 0) {
    return;
  }
  if (millis() - thermoLightTime > THERMO_LIGHT_TIMEOUT) {
    sysConf.transferLEDEvent(LED_POWER_ON);
    thermoLightTime = 0;
  }
}

void bleRecvController(String str) {
  if (str[1] == 0x31) {    // Ctrl
    if (str[2] == 0x30) {  // On / Off Control
      if (str[3] == 0x30) {
        lightOff();
      } else if (str[3] == 0x31) {
        lightOn(LIGHT_CTRL_APP);
      }
    } else if (str[2] == 0x31) {  // User Timer Control
      int sec = str.substring(3, str.length() - 1).toInt();
      userLightTime = millis();
      userLightRuntime = sec * 1000;
      lightOn(LIGHT_CTRL_APP);
    }
  } else if (str[1] == 0x32) {              // Setup
    if (str[2] >= 0x32 && str[2] < 0x37) {  // Change Theme Color
      uint8_t tNum = str[2] - 49;
      uint8_t redValue = str.substring(3, 6).toInt();
      uint8_t greenValue = str.substring(6, 9).toInt();
      uint8_t blueValue = str.substring(9, 12).toInt();
#if DEBUG_LOG
      Serial.printf("Set Theme %d Color : %d, %d, %d\n",
                    tNum, redValue, greenValue, blueValue);
#endif
      themeNum = tNum;
      themeColors[tNum][0] = redValue;
      themeColors[tNum][1] = greenValue;
      themeColors[tNum][2] = blueValue;
      sysConf.transferLEDEvent(LED_POWER_ON);
    }
  } else if (str[1] == 0x32) {              // Check
    if (str[2] >= 0x32 && str[2] < 0x37) {  // Transfer Theme Color
      uint8_t tNum = str[2] - 49;
    }
  }
}


void bleEventHandler() {
  ble_evt_t evtData;
  if (xQueueReceive(ble_queue, &evtData, 10 / portTICK_RATE_MS)) {
    if (evtData.typeNum == BLE_BOOMCARE_CONNECT) {
      // # When the lights are off
      if (!isLightingOn) {
        lightOn(LIGHT_CTRL_DEVICE);
      }
    } else if (evtData.typeNum == BLE_BOOMCARE_DISCONNECT) {
      // # Turns on by connecting a boomcare.
      if (lightCtrlNum == LIGHT_CTRL_DEVICE) {
        lightOff();
      }
    } else if (evtData.typeNum == BLE_MEASURE_TEMPERATURE) {
#if DEBUG_LOG
      Serial.printf("Measure Temperature : %s\n", evtData.recvStr);
#endif
      sysConf.transferLEDEvent(LED_TEMPERATURE_COLOR, false, evtData.recvStr);
      thermoLightTime = millis();
    } else if (evtData.typeNum == BLE_RECV_MESSAGE) {
#if DEBUG_LOG
      Serial.printf("Recv Ctrl Str : %s\n", evtData.recvStr);
#endif
      bleRecvController(evtData.recvStr);
    }
  }
}

void tmpTouchHandler(void* param) {
  while (1) {
    if (!digitalRead(TC1_PIN)) {
      Serial.println("BOTTOM TOUCH.");
      while (!digitalRead(TC1_PIN)) {
        vTaskDelay(50 / portTICK_RATE_MS);
      }
      if (isLightingOn) {
        sysConf.transferLEDEvent(LED_CHANGE_THEME);
      }
    }
    if (!digitalRead(TC2_PIN)) {
      Serial.println("TOP TOUCH.");
      uint8_t pressCnt = 0;
      while (!digitalRead(TC2_PIN)) {
        vTaskDelay(50 / portTICK_RATE_MS);
        if (isLightingOn) {
          pressCnt++;
          if (pressCnt == BTN_LONG_PRESSURE_SIZE) {
            isDimCtrl = !isDimCtrl;
          } else if (pressCnt > BTN_LONG_PRESSURE_SIZE) {
            Serial.println(pressCnt);
            sysConf.transferLEDEvent(LED_BRIGHTNESS_CTRL, isDimCtrl);
          }
        }
      }
      if (pressCnt < BTN_LONG_PRESSURE_SIZE) {
        if (isLightingOn) {
          lightOff();
        } else {
          lightOn(LIGHT_CTRL_SWITCH);
        }
      }
    }
    vTaskDelay(20 / portTICK_RATE_MS);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(TC1_PIN, INPUT_PULLUP);
  pinMode(TC2_PIN, INPUT_PULLUP);

  led_queue = xQueueCreate(2, sizeof(led_ctrl_t));
  ble_queue = xQueueCreate(2, sizeof(ble_evt_t));

  sysConf.begin();
  ble.init();
  led.init();

  led.setState(LED_STA_WIFI_ERROR);
  xTaskCreatePinnedToCore(tmpTouchHandler, "BTN_CTRL_TASK", 1024, NULL, 3, NULL, 0);
}

void loop() {
  bleEventHandler();
  thermoLightTimer();
  userLightTimer();
  delay(10);
}
