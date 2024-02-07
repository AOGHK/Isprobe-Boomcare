#include "Recv.h"

RecvClass Recv;

RecvClass::RecvClass() {
}


void RecvClass::thermoReceiver() {
  thermo_evt_t _evt;
  if (xQueueReceive(thermoQueue, &_evt, 5 / portTICK_RATE_MS)) {
    if (_evt.type == THERMO_CHANGE_CONNECT) {
      Serial.printf("[Recv] :: Boomcare change connect - %d\n", _evt.result);
      Light.thermoConnect(_evt.result);
    } else if (_evt.type == THERMO_MEASURE_RESULT) {
      Serial.printf("[Recv] :: Boomcare measure result - %d\n", _evt.result);
      Light.thermoMeasure(_evt.result);
      // ++ Submit Data
    } else if (_evt.type == THERMO_GET_SOUND_STA) {
      Serial.printf("[Recv] :: Boomcare sound state - %d\n", _evt.result);
    }
  }
}

void RecvClass::bleReceiver() {
  ble_recv_t _recv;
  if (xQueueReceive(bleQueue, &_recv, 5 / portTICK_RATE_MS)) {
    Serial.println(String(_recv.msg));
  }
}


void RecvClass::handle() {
  thermoReceiver();
  bleReceiver();
}