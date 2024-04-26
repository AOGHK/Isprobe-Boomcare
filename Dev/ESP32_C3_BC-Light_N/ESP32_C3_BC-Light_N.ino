#include "Rom.h"
#include "LED.h"
#include "Button.h"
#include "BLE.h"
#include "Battery.h"
#include "WIFI.h"
#include "Proc.h"


void setup() {
  Serial.begin(115200);

  Led.begin();
  Btn.wakeup();

  Bat.init();

  Light.powerSwitch();
  delay(1000);

  BLE.begin();
  mWiFi.begin();
  Btn.task();

  // ESP_LOGE("MAIN", "MY MAC ADDRSS : %s", BLE.getAddress().c_str());
  // Rom.clearTemperature();
}

void loop() {
  if (Serial.available()) {
    int val = Serial.readStringUntil('\n').toInt();
    if (val == 0) {
      mWiFi.renewalData("U+Net6D74,252CA#8JFD");
    } else if (val == 1) {
      uint16_t randNumber = random(3000, 4000);
      mWiFi.upload(HTTP_THERMO_API, BLE.getAddress(), randNumber);
    } else if (val == 2) {
      uint8_t _size = Rom.getTemperatureSize();
      Serial.printf("Backup Temp Size : %d\n", _size);
      temp_date_t* _datetime = (temp_date_t*)malloc(_size * sizeof(temp_date_t));
      temp_value_t* _value = (temp_value_t*)malloc(_size * sizeof(temp_value_t));
      Rom.getTemperature(_datetime, _value);
      for (size_t i = 0; i < _size; i++) {
        Serial.printf("%02d-%02d-%02d  %02d:%02d:%02d -> %02d.%02d\n",
                      _datetime[i].year, _datetime[i].month, _datetime[i].day,
                      _datetime[i].hour, _datetime[i].min, _datetime[i].sec,
                      _value[i].integer, _value[i].point);
      }
      free(_datetime);
      free(_value);
    }
  }

  Proc.run();
  Light.run();
  Bat.scan();

  Proc.ping();
  // Serial.println(ESP.getFreeHeap());
  delay(10);
}
