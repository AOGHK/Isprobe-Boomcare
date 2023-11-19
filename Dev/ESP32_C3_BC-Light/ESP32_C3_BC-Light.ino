#define DEBUG_LOG 1

#include "PinButton.h"
#include "BLE.h"
#include "LED.h"
#include "MyWiFi.h"

#define BOT_TC_PIN 20  // TC1
#define TOP_TC_PIN 21  // TC2

PinButton topBtn(TOP_TC_PIN, INPUT_PULLUP, LOW);
PinButton botBtn(BOT_TC_PIN, INPUT_PULLUP, LOW);
BLE ble;
LED led;
MyWiFi mWiFi;

enum {
  LIGHT_CTRL_SWITCH = 1,
  LIGHT_CTRL_APP,
  LIGHT_CTRL_DEVICE,
};

bool isDimCtrl = true;
bool isLightingOn = false;
uint8_t lightCtrlNum = 0;
unsigned long thermoLightTime = 0;
unsigned long userLightTime = 0;
unsigned long userLightRuntime = 0;
String recvSSID;

#pragma region Action Func
void lightOn(uint8_t ctrlNum) {
  isLightingOn = true;
  lightCtrlNum = ctrlNum;
  thermoLightTime = 0;
  led.ctrlPower(isLightingOn);
}

void lightOff() {
  isLightingOn = false;
  thermoLightTime = 0;
  userLightTime = 0;
  led.ctrlPower(isLightingOn);
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
    led.ctrlPower(true);
    thermoLightTime = 0;
  }
}
#pragma endregion

void bleEventHandler(ble_evt_t data) {
  if (data._type == BLE_CHANGE_CONNECT) {
    Serial.printf("Boomcare Connected : %d\n", data._num);
    if (data._num == 1 && !isLightingOn) {
      lightOn(LIGHT_CTRL_DEVICE);
    } else if (data._num == 0 && lightCtrlNum == LIGHT_CTRL_DEVICE) {
      lightOff();
    }
  } else if (data._type == BLE_MEASURE_TEMPERATURE) {
    Serial.printf("Boomcare Measure Temperature : %d\n", data._num);
    led.setTemperature(data._num);
    thermoLightTime = millis();
  } else if (data._type == BLE_RECV_CTRL_DATA) {
    Serial.printf("Ctrl Data : %s\n", data._str);
    if (data._str[0] == 0x30) {  // On / Off Control
      if (data._str[1] == 0x30) {
        lightOff();
      } else if (data._str[1] == 0x31) {
        lightOn(LIGHT_CTRL_APP);
      }
    } else if (data._str[0] == 0x31) {  // User Timer Control
      uint16_t sec = data._str.substring(1).toInt();
      userLightTime = millis();
      userLightRuntime = sec * 1000;
      lightOn(LIGHT_CTRL_APP);
    }
  } else if (data._type == BLE_RECV_SETUP_DATA) {
    Serial.printf("Setup Data : %s\n", data._str);
    if (data._str[0] >= 0x32 && data._str[0] < 0x37) {  // Change Theme Color
      led.setThemeColor(data._str);
      lightOn(LIGHT_CTRL_APP);
    } else if (data._str[0] == 0x37) {  // Change Brightness
      uint8_t value = data._str.substring(1).toInt();
      led.setBrightness(value);
      lightOn(LIGHT_CTRL_APP);
    } else if (data._str[0] == 0x38) {  // Setup User WiFi SSID
      recvSSID = data._str.substring(1);
    } else if (data._str[0] == 0x39) {  // Setup User WiFi Password
      if (recvSSID.length() != 0) {
        String recvPwd = data._str.substring(1);
        mWiFi.renewalData(recvSSID, recvPwd);
      }
    }
  } else if (data._type == BLE_RECV_REQ_DATA) {
    Serial.printf("Check Data : %s\n", data._str);
    if (data._str[0] >= 0x32 && data._str[0] <= 0x37) {  // Transfer Theme Color & Brightness
      String res = led.getThemeData(data._str[0]);
      ble.writeData(data._str[0], res);
    }
  } else if (data._type == BLE_RECV_REQ_ADDRESS) {
  }
}

void wifiConnectHandler(bool isConn, bool isRenewal) {
  led.setState(isConn);
  if (isRenewal) {
    if (isConn) {
      mWiFi.updateRom();
    }
  }
  Serial.printf("WiFi Connected : %d \n", isConn);
}

void taskTouchEvent(void* param) {
  while (1) {
    topBtn.update();
    botBtn.update();

    if (topBtn.isSingleClick()) {
#if DEBUG_LOG
      Serial.println("[Touch] - Power Ctrl.");
#endif
      isLightingOn = !isLightingOn;
      led.ctrlPower(isLightingOn);
    }

    if (topBtn.isLongClick()) {
#if DEBUG_LOG
      Serial.println("[Touch] - Dimming Ctrl.");
#endif
      isDimCtrl = !isDimCtrl;
      while (1) {
        topBtn.update();
        led.changeBrightness(isDimCtrl);
        if (topBtn.isReleased()) {
          led.saveBrightness();
          break;
        }
        vTaskDelay(50 / portTICK_RATE_MS);
      }
    }
    if (botBtn.isDoubleClick()) {
#if DEBUG_LOG
      Serial.println("[Touch] - Theme Change.");
#endif
      led.changeTheme();
    }
    vTaskDelay(10 / portTICK_RATE_MS);
  }
}

void setup() {
  Serial.begin(115200);

  led.begin();
  ble.begin();
  mWiFi.begin();

  ble.setEvnetCallback(bleEventHandler);
  mWiFi.setConnectCallback(wifiConnectHandler);
  xTaskCreatePinnedToCore(taskTouchEvent, "BTN_CTRL_TASK", 1024 * 2, NULL, 3, NULL, 0);
}

void loop() {
  mWiFi.scanState();

  thermoLightTimer();
  userLightTimer();
  delay(10);
}
