#include "SysConf.h"

uint8_t themeColors[THEME_SIZE + 1][3] = {};
uint8_t themeNum = 0;
uint8_t ledBrightness;
String wifiSSID = "";
String wifiPwd = "";

uint8_t thermoColors[3][3] = {
  { 119, 239, 245 },
  { 175, 244, 52 },
  { 245, 137, 89 }
};

xQueueHandle led_queue;
xQueueHandle ble_queue;
xQueueHandle wifi_queue;

SysConf::SysConf() {
}

void SysConf::begin() {
  if (!EEPROM.begin(EEPROM_SIZE)) {
    while (1) {
      delay(1000);
    }
  }
  if (EEPROM.read(0) != 1) {
    EEPROM.write(0, 1);
    initRom();
  }
  readRom();
}

void SysConf::initRom() {
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
  EEPROM.write(18, 0);  // WiFi SSID Len
  EEPROM.write(19, 0);  // WiFi Pwd Len
  EEPROM.commit();
}

void SysConf::updateThemeNumber() {
  EEPROM.write(2, themeNum);
  EEPROM.commit();
}

void SysConf::updateThemeColor() {
  EEPROM.write(2, themeNum);
  uint8_t startAddr = themeNum * 3;
  EEPROM.write(startAddr, themeColors[themeNum][0]);
  EEPROM.write(startAddr + 1, themeColors[themeNum][1]);
  EEPROM.write(startAddr + 2, themeColors[themeNum][2]);
  EEPROM.commit();
}

void SysConf::updateBrightness() {
  EEPROM.write(1, ledBrightness);
  EEPROM.commit();
}

void SysConf::updateWiFi(String ssid, String pwd) {
  uint8_t ssidLen = ssid.length();
  uint8_t pwdLen = pwd.length();
  EEPROM.write(18, ssidLen);
  EEPROM.write(19, pwdLen);
  for (uint8_t i = 0; i < ssidLen + pwdLen; i++) {
    if (i < ssidLen) {
      EEPROM.write(i + 20, ssid[i]);
    } else {
      EEPROM.write(i + 20, pwd[i - ssidLen]);
    }
  }
  EEPROM.commit();
}

void SysConf::readRom() {
  ledBrightness = EEPROM.read(1);
  themeNum = EEPROM.read(2);

  themeColors[0][0] = 0;
  themeColors[0][1] = 0;
  themeColors[0][2] = 0;

  for (int i = 3; i < 18; i += 3) {
    int pos = i / 3;
    themeColors[pos][0] = EEPROM.read(i);
    themeColors[pos][1] = EEPROM.read(i + 1);
    themeColors[pos][2] = EEPROM.read(i + 2);
#if DEBUG_LOG
    Serial.printf("%d Theme Color : %d, %d, %d\n", pos, themeColors[pos][0], themeColors[pos][1], themeColors[pos][2]);
#endif
  }

  uint8_t ssidLen = EEPROM.read(18);
  uint8_t pwdLen = EEPROM.read(19);
  if (ssidLen != 0) {
    for (uint8_t idx = 0; idx < ssidLen + pwdLen; idx++) {
      if (idx < ssidLen) {
        wifiSSID += (char)EEPROM.read(idx + 20);
      } else {
        wifiPwd += (char)EEPROM.read(idx + 20);
      }
    }
#if DEBUG_LOG
    Serial.print("WiFi Data : ");
    Serial.print(wifiSSID);
    Serial.print(", ");
    Serial.println(wifiPwd);
#endif
  }
}

void SysConf::transferLEDEvent(uint8_t evtNum, bool isDimCtrl, String thermoStr) {
  if (evtNum == LED_CHANGE_THEME) {
    themeNum++;
    if (themeNum > THEME_SIZE) {
      themeNum = 0;
    }
    evtNum = LED_POWER_ON;
  }

  led_ctrl_t lcdCtrl = {};
  if (evtNum == LED_TEMPERATURE_COLOR) {
    evtNum = LED_POWER_ON;
    uint16_t thermo = thermoStr.toInt();
    uint8_t idx;
    if (thermo > 3800) {
      idx = 2;
    } else if (thermo > 3620) {
      idx = 1;
    } else {
      idx = 0;
    }
    lcdCtrl.red = thermoColors[idx][0];
    lcdCtrl.green = thermoColors[idx][1];
    lcdCtrl.blue = thermoColors[idx][2];
    lcdCtrl.brightness = 0;
  } else if (evtNum == LED_POWER_ON) {
    lcdCtrl.red = themeColors[themeNum][0];
    lcdCtrl.green = themeColors[themeNum][1];
    lcdCtrl.blue = themeColors[themeNum][2];
    lcdCtrl.brightness = themeNum == 0 ? ledBrightness : 0;
  } else if (evtNum == LED_BRIGHTNESS_CTRL) {
    uint8_t dimValue = isDimCtrl ? ledBrightness + 1 : ledBrightness - 1;
    if (dimValue <= LED_MAX_BRIGHTNESS && dimValue >= LED_MIN_BRIGHTNESS) {
      ledBrightness = dimValue;
    }
  }
  lcdCtrl.typeNum = evtNum;
  xQueueSend(led_queue, (void*)&lcdCtrl, 10 / portTICK_RATE_MS);
}
