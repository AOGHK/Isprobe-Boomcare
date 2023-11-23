#define DEBUG_LOG 1

#include "Adafruit_MAX1704X.h"
#include "PinButton.h"
#include "BLE.h"
#include "LED.h"
#include "MyWiFi.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#define BOT_TC_PIN 20  // TC1
#define TOP_TC_PIN 21  // TC2
#define SDA_PIN 4
#define SCL_PIN 5
#define SCAN_BATTERY_TIMER 5000

Adafruit_MAX17048 bat;
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

bool isBatEnable = false;
unsigned long scanBatteryTime = 0;
uint8_t batLevel = 0;
String myMacAddress = "";

bool isBridgeMode = false;
uint8_t isBoomcareSound = 0;

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

void scanBatteryLevel() {
  if (!isBatEnable) {
    return;
  }
  if (millis() - scanBatteryTime > SCAN_BATTERY_TIMER) {
    batLevel = bat.cellPercent();
    scanBatteryTime = millis();
  }
}
#pragma endregion

#pragma region Event& Receive Handler
void bleEventHandler(ble_evt_t data) {
  if (data._type == BLE_CHANGE_CONNECT) {
    if (data._num == 1) {
      isBoomcareSound = data._str.toInt();
      if (!isLightingOn) {
        lightOn(LIGHT_CTRL_DEVICE);
      }
    } else if (data._num == 0 && lightCtrlNum == LIGHT_CTRL_DEVICE) {
      lightOff();
    }
  } else if (data._type == BLE_MEASURE_TEMPERATURE) {
    Serial.printf("Measure Thermo Value : %d\n", data._num);
    led.setTemperature(data._num);
    if (isBridgeMode) {
      float tmpValue = (float)data._num / 100;
      ble.writeData('5', String(tmpValue));
    }
    mWiFi.uploadTemperature(data._num);
    thermoLightTime = millis();
  } else if (data._type == BLE_RECV_CTRL_DATA) {
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
    } else if (data._str[0] == 0x42) {  // Bridge Mode Ctrl
      if (data._str[1] == 0x31) {
        if (data._str.substring(2).equals(ble.getBoomcareAddress())) {
          isBridgeMode = true;
        }
        ble.writeData('1', 'B', String(isBridgeMode));
      } else {
        isBridgeMode = false;
      }
    } else if (data._str[0] == 0x43) {  // Boomcare Sound Ctrl
      if (isBridgeMode) {
        isBoomcareSound = data._str[1] - 48;
        xQueueSend(bcQueue, (void*)&isBoomcareSound, 10 / portTICK_RATE_MS);
        Serial.printf("Set Boomcare Sound : %d\n", isBoomcareSound);
      }
    }
  } else if (data._type == BLE_RECV_SETUP_DATA) {
    if (data._str[0] >= 0x32 && data._str[0] < 0x37) {  // Change Theme Color
      led.setThemeColor(data._str);
      lightOn(LIGHT_CTRL_APP);
    } else if (data._str[0] == 0x37) {  // Change Brightness
      uint8_t value = data._str.substring(1).toInt();
      led.setBrightness(value);
      lightOn(LIGHT_CTRL_APP);
    } else if (data._str[0] == 0x38) {  // Setup User WiFi Data
      uint8_t seqPos = data._str.indexOf(',');
      String recvSSID = data._str.substring(1, seqPos);
      String recvPwd = data._str.substring(seqPos + 1, data._str.length());
      mWiFi.renewalData(recvSSID, recvPwd);
    }
  } else if (data._type == BLE_RECV_REQ_DATA) {
    if (data._str[0] >= 0x32 && data._str[0] <= 0x37) {  // Transfer Theme Color & Brightness
      String res = led.getThemeData(data._str[0]);
      ble.writeData('3', data._str[0], res);
    } else if (data._str[0] == 0x43) {  // Boomcare Sound State
      if (isBridgeMode) {
        ble.writeData('3', data._str[0], String(isBoomcareSound));
      }
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
    ble.writeData('2', '8', String(isConn));
  }
}

void taskTouchEvent(void* param) {
  while (1) {
    topBtn.update();
    botBtn.update();

    if (topBtn.isSingleClick()) {
      isLightingOn = !isLightingOn;
      led.ctrlPower(isLightingOn);
    }

    if (topBtn.isLongClick() && isLightingOn) {
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

    if (botBtn.isDoubleClick() && isLightingOn) {
      led.changeTheme();
    }
    vTaskDelay(10 / portTICK_RATE_MS);
  }
}

#pragma endregion

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  isBatEnable = bat.begin();
  // if (!isBatEnable) {
  //   while (1) {
  //     delay(1000);
  //   }
  // }

  led.begin();
  ble.begin();
  mWiFi.begin();
  myMacAddress = ble.getMacAddress();

  Serial.print("My Mac Address : ");
  Serial.println(myMacAddress);

  ble.setEvnetCallback(bleEventHandler);
  mWiFi.setConnectCallback(wifiConnectHandler);
  xTaskCreatePinnedToCore(taskTouchEvent, "BTN_CTRL_TASK", 1024 * 2, NULL, 3, NULL, 0);
}

void loop() {
  thermoLightTimer();
  userLightTimer();

  scanBatteryLevel();

  led.aliveBlink();
  delay(10);
}
