#include "BLE.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"

BLEClass BLE;

const char* BLE_NAME = "BC_LIGHT";
const char* SERVER_SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
const char* SERVER_CHAR_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8";

BLECharacteristic* sCharacteristic = NULL;

xQueueHandle bleQueue = xQueueCreate(2, sizeof(ble_recv_t));

class MyCharCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* sCharacteristic) {
    std::string value = sCharacteristic->getValue();
    if (value.length() <= 0 || value[0] != '$' || value[value.length() - 1] != '#') {
      return;
    }

    ble_recv_t _recv = {};
    for (int i = 2; i < value.length() - 1; i++) {
      _recv.msg[i - 2] += value[i];
    }

    if (value[1] == 0x31) {  // ## Ctrl
      _recv.type = BLE_REMOTE_CTRL;
    } else if (value[1] == 0x32) {  // ## Setup
      _recv.type = BLE_SETUP_ATTR;
    } else if (value[1] == 0x33) {  // ## Check
      _recv.type = BLE_REQ_ATTR;
    } else if (value[1] == 0x34) {  // ## Req Thermo Address
      _recv.type = BLE_REQ_THERMO_ADDRESS;
    }
    xQueueSend(bleQueue, (void*)&_recv, 1 / portTICK_RATE_MS);
  }
};

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* bServer) {
#if DEBUG_LOG
    Serial.printf("[BLE] :: Client Connect.?\n");
#endif
  };
  void onDisconnect(BLEServer* bServer) {
    vTaskDelay(BLES_AD_DELAY / portTICK_RATE_MS);
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
  BLEDevice::init(BLE_NAME);
  startPeripheralMode();
  Thermo.task();

#if DEBUG_LOG
  Serial.printf("[BLE] :: Device Mac Address - %s\n", getAddress().c_str());
#endif
}

String BLEClass::getAddress() {
  if (macAddress == "") {
    const uint8_t* point = esp_bt_dev_get_address();
    char addr[17];
    sprintf(addr, "%02X:%02X:%02X:%02X:%02X:%02X",
            (int)point[0], (int)point[1], (int)point[2],
            (int)point[3], (int)point[4], (int)point[5]);
    macAddress = String(addr);
  }
  return macAddress;
}

void BLEClass::writeStr(String _str) {
  if (sCharacteristic == NULL) {
    return;
  }
  sCharacteristic->setValue(_str.c_str());
  sCharacteristic->notify();
}