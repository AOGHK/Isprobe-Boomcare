#include "SysEnv.h"

#include "BLE.h"
#include "PinButton.h"
#include "MyWiFi.h"
#include "LED.h"
#include "Adafruit_MAX1704X.h"

PinButton topBtn(TOP_TC_PIN, INPUT_PULLUP, LOW, 20);
PinButton botBtn(BOT_TC_PIN, INPUT_PULLUP, LOW);
BLE ble;
MyWiFi mWiFi;
LED led;
Adafruit_MAX17048 bat;

uint8_t BATTERY_LVL = 0;
bool isBatEnable = false;

bool isDimCtrl = true;
bool isLightingOn = false;
bool isThermoCtrl = false;
bool isBridgeMode = false;
bool isUsbConn = false;
bool isWiFiConn = false;

unsigned long thermoLightTime = 0;
unsigned long userLightTime = 0;
unsigned long userLightRuntime = 0;
unsigned long scanBatteryTime = 0;


/* --------------------------------------------
              Light Ctrl
-------------------------------------------- */
void lightOn(bool _isThermoCtrl) {
  isThermoCtrl = _isThermoCtrl;
  thermoLightTime = 0;
  isLightingOn = true;
  led.ctrlPower(isLightingOn);
}

void lightOff() {
  userLightTime = 0;
  isLightingOn = false;
  led.ctrlPower(isLightingOn);
}

/* --------------------------------------------
                Timer Func
-------------------------------------------- */
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
  if (scanBatteryTime == 0 || millis() - scanBatteryTime > SCAN_BATTERY_TIMER) {
    BATTERY_LVL = bat.cellPercent();
    if (BATTERY_LVL > 100) {
      BATTERY_LVL = 100;
    }
    if (!isUsbConn && BATTERY_LVL < LOW_BATTERY_LIMIT) {
      led.lowBattery(LOW_BAT_BLINK_SIZE, LOW_BAT_BLINK_DELAY);
      digitalWrite(PW_CTRL_PIN, LOW);
      while (1) { delay(500); }
    }
    scanBatteryTime = millis();
  }
}


/* --------------------------------------------
                BLE Recever
-------------------------------------------- */
void thermoConnectCtrl(uint8_t _isConn) {
  if (_isConn) {
    if (!isLightingOn) {
      lightOn(true);
    }
  } else if (!_isConn && isThermoCtrl) {
    lightOff();
  }
}

void thermoDataCtrl(uint16_t data) {
  led.setThermoColor(data);
  if (isBridgeMode) {
    float tmpValue = (float)data / 100;
    ble.writeData('5', String(tmpValue));
  }
  mWiFi.uploadThermoValue(data);
  thermoLightTime = millis();
}

void userCommandCtrl(String _Comm) {
  if (_Comm[0] == 0x30) {  // On / Off Control
    if (_Comm[1] == 0x30) {
      lightOff();
    } else if (_Comm[1] == 0x31) {
      lightOn(false);
    }
  } else if (_Comm[0] == 0x31) {  // User Timer Control
    uint16_t sec = _Comm.substring(1).toInt();
    userLightTime = millis();
    userLightRuntime = sec * 1000;
    lightOn(false);
  } else if (_Comm[0] == 0x42) {  // Bridge Mode Ctrl
    if (_Comm[1] == 0x31) {
      if (ble.isSameThermo(_Comm.substring(2))) {
        isBridgeMode = true;
      }
    } else {
      isBridgeMode = false;
    }
    ble.writeData('1', 'B', String(isBridgeMode));
  } else if (_Comm[0] == 0x43) {  // Boomcare Sound Ctrl
    if (isBridgeMode) {
      uint8_t sta = _Comm[1] - 48;
      ble_evt_t evtData = {
        ._type = BLEC_CHANGE_SOUND_STA,
        ._num = sta,
      };
      xQueueSend(blecQueue, (void*)&evtData, 10 / portTICK_RATE_MS);
    }
  }
}

void setUserPreferences(String _str) {
  if (_str[0] >= 0x32 && _str[0] < 0x37) {  // Change Theme Color
    led.setThemeColor(_str);
    lightOn(false);
  } else if (_str[0] == 0x37) {  // Change Brightness
    uint8_t value = _str.substring(1).toInt();
    led.setBrightness(value);
    lightOn(false);
  } else if (_str[0] == 0x38) {  // Setup User WiFi Data
    uint8_t seqPos = _str.indexOf(',');
    String recvSSID = _str.substring(1, seqPos);
    String recvPwd = _str.substring(seqPos + 1, _str.length());
    mWiFi.renewalConnect(recvSSID, recvPwd);
  }
}

void transferPreferences(String _str) {
  if (_str[0] >= 0x32 && _str[0] <= 0x37) {  // Transfer Theme Color & Brightness
    String res = led.getPreferences(_str[0]);
    ble.writeData('3', _str[0], res);
  } else if (_str[0] == 0x43) {  // Boomcare Sound State
    if (isBridgeMode) {
      ble.transferSoundState();
    }
  }
}

void bleEvtHandler(ble_evt_t data) {
  if (data._type == BLEC_SCAN_DISCOVERY) {
    // 
  } else if (data._type == BLEC_CHANGE_CONNECT) {
    thermoConnectCtrl(data._num);
  } else if (data._type == BLEC_RES_TEMPERATURE) {
    thermoDataCtrl(data._num);
  } else if (data._type == BLES_RECV_CTRL_DATA) {
    userCommandCtrl(data._str);
  } else if (data._type == BLES_RECV_SETUP_DATA) {
    setUserPreferences(data._str);
  } else if (data._type == BLES_RECV_REQ_DATA) {
    transferPreferences(data._str);
  } else if (data._type == BLES_RECV_REQ_ADDRESS) {
    ble.transferMyAddress();
  }
}


/* --------------------------------------------
                WiFi Recever
-------------------------------------------- */
void wifiStateHandler(bool _isConnected, bool _isConnRenewal) {
#ifdef DEBUG_LOG
  Serial.printf("[WiFi] :: Connect Sta : %d\n", _isConnected);
#endif
  isWiFiConn = _isConnected;
  if (!isUsbConn) {
    led.setState(isWiFiConn ? LED_STA_WIFI_CONN : LED_STA_WIFI_DISCONN);
  }
  if (_isConnRenewal) {
    if (isWiFiConn) {
      mWiFi.updateRom();
    }
    ble.writeData('2', '8', String(isWiFiConn));
  }
}

/* --------------------------------------------
              Button Evt Handler
-------------------------------------------- */
void powerCtrl() {
  bool isUsb = digitalRead(PW_STA_PIN);
  if (isUsbConn != isUsb) {
    led.setState(isUsb ? LED_STA_CHARGE : (isWiFiConn ? LED_STA_WIFI_CONN : LED_STA_WIFI_DISCONN));
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

void taskBtnHandler(void* param) {
  while (1) {
    powerCtrl();

    topBtn.update();
    botBtn.update();
    if (topBtn.isSingleClick()) {
      isLightingOn = !isLightingOn;
      if (isLightingOn) {
        lightOn(false);
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

void setup() {
#if DEBUG_LOG
  Serial.begin(115200);
  Serial.println();
#endif
  led.begin();

  // @ Power On.
  pinMode(PW_STA_PIN, INPUT_PULLUP);
  pinMode(PW_BTN_PIN, INPUT_PULLDOWN);
  pinMode(PW_CTRL_PIN, OUTPUT);
  digitalWrite(PW_CTRL_PIN, HIGH);

  // @ Init Sta Led
  isUsbConn = digitalRead(PW_STA_PIN);
  led.setState(isUsbConn ? LED_STA_CHARGE : LED_STA_WIFI_DISCONN);
  while (digitalRead(PW_BTN_PIN)) { delay(10); }

  // @ Check Battery
  Wire.begin(SDA_PIN, SCL_PIN);
  isBatEnable = bat.begin();
  delay(100);
  scanBatteryLevel();

  // @ Init LED
  led.startAct();
  isLightingOn = true;

  // @ Set Wireless & Button
  ble.begin();
  mWiFi.begin();
  ble.setCallback(bleEvtHandler);
  mWiFi.setCallback(wifiStateHandler);
  // xTaskCreatePinnedToCore(taskBtnHandler, "BTN_CTRL_TASK", 1024 * 2, NULL, 3, NULL, 1);
  xTaskCreate(taskBtnHandler, "BTN_CTRL_TASK", 1024 * 2, NULL, 3, NULL);
}

void loop() {
  thermoLightTimer();
  userLightTimer();
  scanBatteryLevel();
  delay(10);
}
