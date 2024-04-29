#include "BLE.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"

BLEClass BLE;

BLECharacteristic* sCharacteristic = NULL;
const char* SERVER_SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
const char* SERVER_CHAR_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8";

xQueueHandle bleQueue = xQueueCreate(2, sizeof(ble_recv_t));

class MyCharCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* sCharacteristic) {
    std::string value = sCharacteristic->getValue();
    if (value.length() <= 0 || value[0] != 0x24 || value[value.length() - 1] != 0x23) {
      return;
    }
    ble_recv_t _data = {
      .header = value[1],
      .len = value.length() - 3
    };
    _data.cmd = (uint8_t*)malloc(_data.len * sizeof(uint8_t));
    for (int i = 2; i < value.length() - 1; i++) {
      _data.cmd[i - 2] = value[i];
    }
    xQueueSend(bleQueue, (void*)&_data, 1 / portTICK_RATE_MS);
  }
};

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* bServer){

  };

  void onDisconnect(BLEServer* bServer) {
    vTaskDelay(100 / portTICK_RATE_MS);
    BLEDevice::startAdvertising();
  }
};

void BLEClass::startPeripheralMode() {
  BLEServer* bleServer = BLEDevice::createServer();
  bleServer->setCallbacks(new MyServerCallbacks());  // Server

  BLEService* bleService = bleServer->createService(SERVER_SERVICE_UUID);
  sCharacteristic = bleService->createCharacteristic(SERVER_CHAR_UUID, BLECharacteristic::PROPERTY_READ
                                                                         | BLECharacteristic::PROPERTY_WRITE
                                                                         | BLECharacteristic::PROPERTY_NOTIFY);
  sCharacteristic->addDescriptor(new BLE2902());
  sCharacteristic->setCallbacks(new MyCharCallbacks());
  bleService->start();

  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVER_SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x12);

  BLEDevice::startAdvertising();
}

BLEClass::BLEClass() {
}

void BLEClass::begin() {
  BLEDevice::init("BC_LIGHT");
  startPeripheralMode();
  Thermo.task();
}

uint8_t* BLEClass::getAddress() {
  if (macAddress[0] == 0) {
    const uint8_t* point = esp_bt_dev_get_address();
    memcpy(macAddress, point, 6);
  }
  ESP_LOGE("BLE", "MY MAC ADDRSS : %02X:%02X:%02X:%02X:%02X:%02X",
           macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5]);
  return macAddress;
}


void BLEClass::write(uint8_t _header, uint8_t _value) {
  if (sCharacteristic == NULL) {
    return;
  }
  uint8_t _data[4] = { 36, _header, _value, 35 };
  sCharacteristic->setValue(_data, 4);
  sCharacteristic->notify();
}


void BLEClass::write(uint8_t* _data, uint8_t _len) {
  if (sCharacteristic == NULL) {
    return;
  }
  sCharacteristic->setValue(_data, _len);
  sCharacteristic->notify();
}
