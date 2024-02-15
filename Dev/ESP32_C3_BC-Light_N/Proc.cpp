#include "Proc.h"

ProcClass Proc;

ProcClass::ProcClass() {
}

void ProcClass::thermoReceiver() {
  thermo_evt_t _evt;
  if (xQueueReceive(thermoQueue, &_evt, 1 / portTICK_RATE_MS)) {
    if (_evt.type == THERMO_CHANGE_CONNECT) {
#if DEBUG_LOG
      Serial.printf("[Recv] :: Boomcare change connect - %d\n", _evt.result);
#endif
      Light.thermoConnect(_evt.result);
      if (_evt.result == 0) {
        isBridgeMode = false;
      }
    } else if (_evt.type == THERMO_MEASURE_RESULT) {
#if DEBUG_LOG
      Serial.printf("[Recv] :: Boomcare measure result - %d\n", _evt.result);
#endif
      Light.thermoMeasure(_evt.result);
      mWiFi.uploadThermo(BLE.getAddress(), _evt.result);
      if (isBridgeMode) {
        float tempValue = (float)_evt.result / 100;
        String str = "$5" + String(tempValue) + "#";
        BLE.writeStr(str);
      }
    } else if (_evt.type == THERMO_GET_SOUND_STA) {
#if DEBUG_LOG
      Serial.printf("[Recv] :: Boomcare sound state - %d\n", _evt.result);
#endif
    }
  }
}

void ProcClass::wifiReceiver() {
  bool _isConn;
  if (xQueueReceive(wifiConnQueue, &_isConn, 1 / portTICK_RATE_MS)) {
    String str = "$28" + String(_isConn) + "#";
    BLE.writeStr(str);
  }
}

void ProcClass::bleReceiver() {
  ble_recv_t _recv;
  if (xQueueReceive(bleQueue, &_recv, 1 / portTICK_RATE_MS)) {
    if (_recv.type == BLE_REMOTE_CTRL) {
      remoteCtrl(_recv.msg);
    } else if (_recv.type == BLE_SETUP_ATTR) {
      userSettings(_recv.msg);
    } else if (_recv.type == BLE_REQ_ATTR) {
      submitAttribute(_recv.msg);
    }
  }
}

void ProcClass::handle() {
  thermoReceiver();
  bleReceiver();
  wifiReceiver();
}


void ProcClass::setBridgeMode(String _cmd) {
  if (_cmd[1] != 0x31) {
    isBridgeMode = false;
  } else {
    isBridgeMode = Thermo.isSameDevice(_cmd.substring(2));
  }
  String str = "$1B" + String(isBridgeMode) + "#";
  BLE.writeStr(str);
}

void ProcClass::remoteCtrl(String _cmd) {
  if (_cmd[0] == 0x30) {  // On / Off Control
    Light.powerSwitch(_cmd[1] == 0x31);
  } else if (_cmd[0] == 0x31) {  // User Timer Control
    uint16_t sec = _cmd.substring(1).toInt();
    Light.startUserTimer(sec);
  } else if (_cmd[0] == 0x42) {  // Bridge Mode Ctrl
    setBridgeMode(_cmd);
  } else if (_cmd[0] == 0x43) {  // Boomcare Sound Ctrl
    if (isBridgeMode) {
      Thermo.setSoundState(_cmd[1] - 48);
    } else {
      BLE.writeStr("$1B0#");
    }
  }
}

void ProcClass::userSettings(String _cmd) {
  if (_cmd[0] >= 0x32 && _cmd[0] < 0x37) {  // Change Theme Color
    Light.setActivate(true);
    Led.setThemeColor(_cmd);
  } else if (_cmd[0] == 0x37) {  // Change Brightness
    uint8_t _brightness = _cmd.substring(2).toInt();
    Light.setActivate(true);
    Led.setBrightness(_brightness, _cmd[1] - 48);
  } else if (_cmd[0] == 0x38) {  // Setup User WiFi Data
    mWiFi.renewalData(_cmd.substring(1));
  }
}

void ProcClass::submitAttribute(String _cmd) {
  String attrStr = "$3" + String(_cmd[0]);
  if (_cmd[0] >= 0x32 && _cmd[0] < 0x37) {  // Theme Color
    attrStr += Led.getThemeColor(_cmd[0] - 49);
  } else if (_cmd[0] == 0x30) {  // Light State
    attrStr += String(Light.getActivate());
  } else if (_cmd[0] == 0x37) {  // Brightness
    attrStr += String(Led.getBrightness());
  } else if (_cmd[0] == 0x38) {  // WiFi State
    attrStr += String(mWiFi.isConnected());
  } else if (_cmd[0] == 0x42) {  // Battery Level
    attrStr += String(Bat.getLevel());
  } else if (_cmd[0] == 0x54) {  // Theme Number
    attrStr += String(Led.getThemeNumber());
  } else if (_cmd[0] == 0x43) {  // Boomcare Sound State
    attrStr += String(Thermo.getSoundState());
  } else if (_cmd[0] == 0x41) {  // All State
    attrStr += String(Light.getActivate())
               + String(Led.getThemeNumber())
               + String(mWiFi.isConnected())
               + String(Bat.getLevel());
  }
  attrStr += "#";
  // #if DEBUG_LOG
  //   Serial.printf("[Recv] :: Attribute Str - %s\n", attrStr.c_str());
  // #endif
  BLE.writeStr(attrStr);
}

void ProcClass::ping() {
  if (!mWiFi.isConnected()) {
    return;
  }

  if (millis() - syncPingTime > WIFI_PING_TIMER) {
    mWiFi.syncPing(BLE.getAddress(), Bat.getLevel());
    syncPingTime = millis();
  }
}