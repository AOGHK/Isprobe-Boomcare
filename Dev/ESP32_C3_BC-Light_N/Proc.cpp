#include "Proc.h"

ProcClass Proc;

xQueueHandle staEventQueue = xQueueCreate(3, sizeof(sta_evt_t));

ProcClass::ProcClass() {
}

void ProcClass::run() {
  syncDotLed();
  stateEventHandle();
  bleReceiveHandle();
}

void ProcClass::syncDotLed() {
  uint32_t _color = digitalRead(PW_STA_PIN) ? DOT_GREEN_COLOR : (mWiFi.isConnected() ? DOT_BLUE_COLOR : DOT_RED_COLOR);
  if (dotColor == _color) {
    return;
  }
  if (dotColor == DOT_GREEN_COLOR) {
    Bat.resetTime();
  }
  dotColor = _color;
  Led.setDot(dotColor);
}

void ProcClass::sendEvtQueue(uint8_t _type, uint16_t _data) {
  sta_evt_t evt = {
    .type = _type,
    .data = _data,
  };
  xQueueSend(staEventQueue, (void*)&evt, 1 / portTICK_RATE_MS);
}

void ProcClass::stateEventHandle() {
  sta_evt_t _evt;
  if (xQueueReceive(staEventQueue, &_evt, 1 / portTICK_RATE_MS)) {
    if (_evt.type == THERMO_DISCOVERY) {
      ESP_LOGE(PROC_TAG, "Boomcare Discovery");
      Light.thermoConnection(true);
    } else if (_evt.type == THERMO_CHANGE_CONNECT) {
      ESP_LOGE(PROC_TAG, "Boomcare Change Connect - %d", _evt.data);
      if (!_evt.data) {
        Light.thermoConnection(false);
        isBridgeMode = false;
      }
      writeThermometerState();
    } else if (_evt.type == THERMO_MEASURE_RESULT) {
      ESP_LOGE(PROC_TAG, "Boomcare Measure Value - %d", _evt.data);
      mTemperature = _evt.data;
      if (Light.isActivated()) {
        Light.thermoMeasurement(mTemperature);
      } else {
        mWiFi.upload(HTTP_THERMO_API, BLE.getAddress(), mTemperature);
      }
    } else if (_evt.type == THERMO_GET_SOUND_STA) {
      ESP_LOGE(PROC_TAG, "Boomcare Sound State - %d", _evt.data);
    } else if (_evt.type == WIFI_CONNECT_RESULT) {
      ESP_LOGE(PROC_TAG, "WiFi Connect Result - %d", _evt.data);
    } else if (_evt.type == BTN_CHANGE_POWER_STA) {
      BLE.write(0x55, Light.isActivated());
    } else if (_evt.type == BTN_CHANGE_THEME_NUM) {
      BLE.write(0x56, Led.getThemeNumber());
    } else if (_evt.type == BTN_CHANGE_LED_BRIGHTNESS) {
      BLE.write(0x57, Led.getBrightness());
    } else if (_evt.type == LED_THERMO_RGB_TIMEOUT) {
      mWiFi.upload(HTTP_THERMO_API, BLE.getAddress(), mTemperature);
    }
  }
}


void ProcClass::bleReceiveHandle() {
  ble_recv_t _data;
  if (xQueueReceive(bleQueue, &_data, 1 / portTICK_RATE_MS)) {

    // Serial.print(_data.header, HEX);
    // Serial.print(" -> ");
    // for (uint8_t i = 0; i < _data.len; i++) {
    //   Serial.print(_data.cmd[i], HEX);
    //   Serial.print(" ");
    // }
    // Serial.println();

    if (_data.header == 0x30 && _data.len == 1) {
      Light.powerSwitch(_data.cmd[0]);
    } else if (_data.header == 0x31 && _data.len == 2) {
      uint16_t _sec = (_data.cmd[0] << 8) + _data.cmd[1];
      ESP_LOGE(PROC_TAG, "Start User Timer : %d", _sec);
      Light.setUserTimer(_sec);
    } else if (_data.header == 0x33 && _data.len == 1) {
      Thermo.setSoundState(_data.cmd[0]);
    } else if (_data.header == 0x40) {
      Light.changeThemeColor(_data.cmd[0], _data.cmd[2], _data.cmd[3], _data.cmd[4], _data.cmd[1]);
    } else if (_data.header == 0x41 && _data.len == 2) {
      Light.setBrightness(_data.cmd[1], _data.cmd[0]);
    } else if (_data.header == 0x42) {
      String _str = String((char*)_data.cmd, _data.len);
      mWiFi.renewalData(_str);
    } else if (_data.header == 0x51) {
      writeLightState();
    } else if (_data.header == 0x52) {
      writeThermometerState();
    } else if (_data.header == 0x50) {
      writeLightState();
      delay(50);
      writeThermometerState();
    }
  }
}

void ProcClass::writeThermometerState() {
  if (!Thermo.isConnected()) {
    BLE.write(0x52, 0);
  } else {
    uint8_t _sta[11];
    _sta[0] = 36;
    _sta[1] = 0x52;
    _sta[2] = 1;
    _sta[3] = Thermo.getSoundState();
    uint8_t* addrs = Thermo.getAddress();
    for (size_t idx = 0; idx < 6; idx++) {
      _sta[idx + 4] = addrs[idx];
    }
    _sta[10] = 35;
    BLE.write(_sta, 11);
    delay(50);
    BLE.write(0x55, Light.isActivated());
  }
}

void ProcClass::writeLightState() {
  uint8_t _sta[17];
  _sta[0] = 36;
  _sta[1] = 0x51;
  _sta[2] = Light.isActivated();
  _sta[3] = mWiFi.isConnected();
  _sta[4] = Bat.getLevel();
  _sta[5] = Led.getThemeNumber();
  _sta[6] = Led.getBrightness();

  led_theme_t* colors = Led.getThemeColor();
  for (size_t idx = 0; idx < 3; idx++) {
    uint8_t pos = idx * 3;
    _sta[pos + 7] = colors[idx].red;
    _sta[pos + 8] = colors[idx].green;
    _sta[pos + 9] = colors[idx].blue;
  }
  _sta[16] = 35;
  BLE.write(_sta, 17);
}

void ProcClass::ping() {
  if (!mWiFi.isConnected()) {
    return;
  }
  if (syncPingTime == 0 || millis() - syncPingTime > WIFI_REQ_PING_TIMER) {
    mWiFi.upload(HTTP_PING_API, BLE.getAddress(), Bat.getLevel());
    syncPingTime = millis();
  }
}
