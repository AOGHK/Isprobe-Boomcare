#include "SysConf.h"
#include "BLE.h"
#include "LED.h"
#include "PinButton.h"
#include "MyWiFi.h"
#include "time.h"

SysConf sysConf;
BLE ble;
LED led;
MyWiFi mWiFi;

PinButton topBtn(TC2_PIN, INPUT_PULLUP, LOW);
PinButton botBtn(TC1_PIN, INPUT_PULLUP, LOW);

const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
uint8_t timeZone = 9;
uint8_t summerTime = 0;  // 3600


bool isDimCtrl = true;
bool isLightingOn = false;
uint8_t lightCtrlNum = 0;
unsigned long thermoLightTime = 0;
unsigned long userLightTime = 0;
unsigned long userLightRuntime = 0;
String recvSSID = "";
String recvPwd = "";

/* =================================
             Utils
================================= */
String zeroPad(uint8_t value) {
  if (value < 10) {
    return "00" + String(value);
  } else if (value < 100) {
    return "0" + String(value);
  } else {
    return String(value);
  }
}

void syncNTPTime() {
  configTime(3600 * timeZone, 3600 * summerTime, ntpServer1, ntpServer2);
  getLocalTime();
}

String getLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
#if DEBUG_LOG
    Serial.println("No time available (yet)");
#endif
    return "Unknown";
  }
  int yy = timeinfo.tm_year + 1900;
  int MM = timeinfo.tm_mon + 1;
  int dd = timeinfo.tm_mday;
  int hh = timeinfo.tm_hour;
  int mm = timeinfo.tm_min;
  int ss = timeinfo.tm_sec;
  String timeStr = String(yy % 100) + "-" + String(MM) + "-" + String(dd) + " "
                   + String(hh) + ":" + String(mm) + "-" + String(ss);
#if DEBUG_LOG
  Serial.print("KST Time : ");
  Serial.println(timeStr);
#endif
  return timeStr;
}

/* =================================
            Action Func
================================= */
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

/* =================================
            Data Handler
================================= */
void bleRecvController(String str) {
  if (str[1] == 0x31) {    // ### Ctrl
    if (str[2] == 0x30) {  // On / Off Control
      if (str[3] == 0x30) {
        lightOff();
      } else if (str[3] == 0x31) {
        lightOn(LIGHT_CTRL_APP);
      }
    } else if (str[2] == 0x31) {  // User Timer Control
      uint16_t sec = str.substring(3, str.length() - 1).toInt();
#if DEBUG_LOG
      Serial.printf("Set User Timer : %d\n", sec);
#endif
      userLightTime = millis();
      userLightRuntime = sec * 1000;
      lightOn(LIGHT_CTRL_APP);
    }
  } else if (str[1] == 0x32) {              // ### Setup
    if (str[2] >= 0x32 && str[2] < 0x37) {  // Change Theme Color
      uint8_t tNum = str[2] - 49;
      themeColors[tNum][0] = str.substring(3, 6).toInt();
      themeColors[tNum][1] = str.substring(6, 9).toInt();
      themeColors[tNum][2] = str.substring(9, 12).toInt();
      ;
#if DEBUG_LOG
      Serial.printf("Set Theme %d Color : %d, %d, %d\n",
                    tNum, themeColors[tNum][0], themeColors[tNum][1], themeColors[tNum][2]);
#endif
      themeNum = tNum;
      lightOn(LIGHT_CTRL_APP);
      sysConf.updateThemeColor();
    } else if (str[2] == 0x37) {  // Change Brightness
      uint8_t brightness = str.substring(3, str.length() - 1).toInt();
#if DEBUG_LOG
      Serial.printf("Set LED  Brightness: %d\n", brightness);
#endif
      themeNum = 0;
      ledBrightness = brightness;
      lightOn(LIGHT_CTRL_APP);
      sysConf.updateBrightness();
    } else if (str[2] == 0x38) {  // Setup User WiFi SSID
      recvSSID = str.substring(3, str.length() - 1);
#if DEBUG_LOG
      Serial.print("Recv WiFi SSID : ");
      Serial.println(recvSSID);
#endif
    } else if (str[2] == 0x39) {  // Setup User WiFi Password
      recvPwd = str.substring(3, str.length() - 1);
#if DEBUG_LOG
      Serial.print("Recv WiFi Password : ");
      Serial.println(recvPwd);
#endif
      if (recvSSID.length() != 0) {
        mWiFi.renewalData(recvSSID, recvPwd);
      }
    }
  } else if (str[1] == 0x33) {              // ### Check
    if (str[2] >= 0x32 && str[2] < 0x37) {  // Transfer Theme Color
      uint8_t tNum = str[2] - 49;
      String res = zeroPad(themeColors[tNum][0]) + zeroPad(themeColors[tNum][1]) + zeroPad(themeColors[tNum][2]);
      ble.writeStr(str[1], str[2], res);
    } else if (str[2] == 0x37) {  // Transfer Brightness
      ble.writeStr(str[1], str[2], String(ledBrightness));
    }
  } else if (str[1] == 0x34) {  // Request Mac Address
    ble.writeStr(str[1], 0x00, ble.getMacAddress());
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
    }
  }
}

void wifiEventHandler() {
  wifi_evt_t evtData;
  if (xQueueReceive(wifi_queue, &evtData, 10 / portTICK_RATE_MS)) {
    if (evtData.typeNum == WIFI_CHANGE_CONNECT) {
#if DEBUG_LOG
      Serial.printf("WiFi Connected : %d\n", evtData.isConnected);
#endif
      if (evtData.isConnected) {
        led.setState(LED_STA_WIFI_CONNECTED);
        syncNTPTime();
        if (evtData.isUpdate) {
          ble.writeStr(0x32, 0x41, "1");
          sysConf.updateWiFi(recvSSID, recvPwd);
        }
      } else {
        led.setState(LED_STA_WIFI_DISCONNECT);
        if (evtData.isUpdate) {
          ble.writeStr(0x32, 0x41, "0");
        }
      }
    } else if (evtData.typeNum == WIFI_TMP_UPLOAD_ERR) {
    }
  }
}

/* =================================
            Touch Event
================================= */
void touch_event_task(void* param) {
  while (1) {
    topBtn.update();
    botBtn.update();

    if (topBtn.isSingleClick()) {
#if DEBUG_LOG
      Serial.println("Power Ctrl.");
#endif
      if (isLightingOn) {
        lightOff();
      } else {
        lightOn(LIGHT_CTRL_SWITCH);
      }
    }

    if (topBtn.isLongClick()) {
#if DEBUG_LOG
      Serial.println("Dimming Ctrl.");
#endif
      if (isLightingOn && themeNum == 0) {
        isDimCtrl = !isDimCtrl;
        while (1) {
          topBtn.update();
          sysConf.transferLEDEvent(LED_BRIGHTNESS_CTRL, isDimCtrl);
          if (topBtn.isReleased()) {
            sysConf.updateBrightness();
            break;
          }
          vTaskDelay(50 / portTICK_RATE_MS);
        }
      }
    }

    if (botBtn.isDoubleClick()) {
#if DEBUG_LOG
      Serial.println("Theme Ctrl.");
#endif
      if (isLightingOn) {
        sysConf.transferLEDEvent(LED_CHANGE_THEME);
        sysConf.updateThemeNumber();
      }
    }
    vTaskDelay(10 / portTICK_RATE_MS);
  }
}

/* =================================
            Main Func
================================= */
void setup() {
  Serial.begin(115200);

  pinMode(TC1_PIN, INPUT_PULLUP);
  pinMode(TC2_PIN, INPUT_PULLUP);

  led_queue = xQueueCreate(2, sizeof(led_ctrl_t));
  ble_queue = xQueueCreate(2, sizeof(ble_evt_t));
  wifi_queue = xQueueCreate(2, sizeof(wifi_evt_t));

  sysConf.begin();

  ble.init();
  led.init();
  mWiFi.init();

  ble.setBleReceiveCallback(bleRecvController);
  xTaskCreatePinnedToCore(touch_event_task, "BTN_CTRL_TASK", 1024 * 2, NULL, 3, NULL, 0);

  led.setState(LED_STA_WIFI_DISCONNECT);
}

void loop() {
  bleEventHandler();
  wifiEventHandler();

  thermoLightTimer();
  userLightTimer();

  mWiFi.sync();

  delay(10);
}
