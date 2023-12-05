// #define DEBUG_LOG

#include "Adafruit_MAX1704X.h"
#include "PinButton.h"
#include "BLE.h"
#include "LED.h"
#include "MyWiFi.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#define PW_STA_PIN 7
#define PW_BTN_PIN 10
#define PW_CTRL_PIN 8
#define BOT_TC_PIN 20  // TC1
#define TOP_TC_PIN 21  // TC2
#define SDA_PIN 4
#define SCL_PIN 5
#define SCAN_BATTERY_TIMER 5000

Adafruit_MAX17048 bat;
PinButton topBtn(TOP_TC_PIN, INPUT_PULLUP, LOW, 20);
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
uint8_t lightCtrlNum = LIGHT_CTRL_SWITCH;
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

bool isUsbConn = false;
bool isWiFiConn = false;

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
    if (batLevel > 100) {
      batLevel = 100;
    }
    scanBatteryTime = millis();
  }
}
#pragma endregion

#pragma region Event& Receive Handler
void bleEventHandler(ble_evt_t data) {
  if (data._type == BLEC_SCAN_DISCOVERY) {
    if (!isLightingOn) {
      lightOn(LIGHT_CTRL_DEVICE);
    }
  } else if (data._type == BLEC_CHANGE_CONNECT) {
#ifdef DEBUG_LOG
    Serial.printf("Boomcare Connected : %d, Light STA : %d\n", data._num, isLightingOn);
#endif
    if (data._num == 1) {
      if (!isLightingOn) {
        lightOn(LIGHT_CTRL_DEVICE);
      }
    } else if (data._num == 0 && lightCtrlNum == LIGHT_CTRL_DEVICE) {
      lightOff();
    }
  } else if (data._type == BLEC_RES_TEMPERATURE) {
#ifdef DEBUG_LOG
    Serial.printf("Measure Thermo Value : %d\n", data._num);
#endif
    led.setTemperature(data._num);
    if (isBridgeMode) {
      float tmpValue = (float)data._num / 100;
      ble.writeData('5', String(tmpValue));
    }
    mWiFi.uploadTemperature(data._num);
    thermoLightTime = millis();
  } else if (data._type == BLEC_RES_STA_SOUND) {
    isBoomcareSound = data._num;
#ifdef DEBUG_LOG
    Serial.printf("Boomcare Sound State : %d\n", isBoomcareSound);
#endif
  } else if (data._type == BLES_RECV_CTRL_DATA) {
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
      } else {
        isBridgeMode = false;
      }
      ble.writeData('1', 'B', String(isBridgeMode));
    } else if (data._str[0] == 0x43) {  // Boomcare Sound Ctrl
      if (isBridgeMode) {
        isBoomcareSound = data._str[1] - 48;
        xQueueSend(bcQueue, (void*)&isBoomcareSound, 10 / portTICK_RATE_MS);
      }
    }
  } else if (data._type == BLES_RECV_SETUP_DATA) {
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
  } else if (data._type == BLES_RECV_REQ_DATA) {
    if (data._str[0] >= 0x32 && data._str[0] <= 0x37) {  // Transfer Theme Color & Brightness
      String res = led.getThemeData(data._str[0]);
      ble.writeData('3', data._str[0], res);
    } else if (data._str[0] == 0x43) {  // Boomcare Sound State
      if (isBridgeMode) {
        ble.writeData('3', data._str[0], String(isBoomcareSound));
      }
    }
  } else if (data._type == BLES_RECV_REQ_ADDRESS) {
  }
}

void wifiConnectHandler(bool isConn, bool isRenewal) {
  isWiFiConn = isConn;
  if (!isUsbConn) {
    led.setWiFiState(isConn);
  }

#ifdef DEBUG_LOG
  Serial.printf("WiFi Connected : %d\n", isConn);
#endif
  if (isRenewal) {
    if (isConn) {
      mWiFi.updateRom();
    }
    ble.writeData('2', '8', String(isConn));
  }
}

void checkPowerCtrl() {
  bool isUsb = digitalRead(PW_STA_PIN);
  if (isUsbConn != isUsb) {
    if (isUsb) {
      led.setUsbState();
    } else {
      led.setWiFiState(isWiFiConn);
    }
    isUsbConn = isUsb;
  }  
  if (isUsb) {
    return;
  }

  if (digitalRead(PW_BTN_PIN)) {
    uint8_t pressCnt = 0;
    while (digitalRead(PW_BTN_PIN)) {
      pressCnt++;
      if (pressCnt == 5) {
        led.clear();
      }
      vTaskDelay(50 / portTICK_RATE_MS);
    }
    if (pressCnt >= 5) {
      digitalWrite(PW_CTRL_PIN, LOW);
    }
  }
}

void taskTouchEvent(void* param) {
  while (1) {
    checkPowerCtrl();

    topBtn.update();
    botBtn.update();
    if (topBtn.isSingleClick()) {
      isLightingOn = !isLightingOn;
      if (isLightingOn) {
        lightOn(LIGHT_CTRL_SWITCH);
      } else {
        lightOff();
      }
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
        vTaskDelay(30 / portTICK_RATE_MS);
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
#ifdef DEBUG_LOG
  Serial.begin(115200);
  Serial.println();
#endif
  led.begin();
  // @ Power On.
  pinMode(PW_STA_PIN, INPUT_PULLUP);
  pinMode(PW_BTN_PIN, INPUT_PULLDOWN);
  pinMode(PW_CTRL_PIN, OUTPUT);
  digitalWrite(PW_CTRL_PIN, HIGH);

  isUsbConn = digitalRead(PW_STA_PIN);
  if (isUsbConn) {
    led.setUsbState();
  } else {
    led.setWiFiState(false);
  }

  if (digitalRead(PW_BTN_PIN)) {
    while (digitalRead(PW_BTN_PIN)) {
      delay(10);
    }
  }

  led.initAction();
  isLightingOn = true;
  // @ init Modules
  Wire.begin(SDA_PIN, SCL_PIN);
  isBatEnable = bat.begin();
  ble.begin();
  mWiFi.begin();
  myMacAddress = ble.getMacAddress();
  // @ Set Callback & Task.
  ble.setEvnetCallback(bleEventHandler);
  mWiFi.setConnectCallback(wifiConnectHandler);
  xTaskCreatePinnedToCore(taskTouchEvent, "BTN_CTRL_TASK", 1024 * 2, NULL, 3, NULL, 0);
}

void loop() {
  thermoLightTimer();
  userLightTimer();
  scanBatteryLevel();
  // led.aliveBlink();
  delay(10);
}
