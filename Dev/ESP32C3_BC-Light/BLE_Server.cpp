#include "BLE_Server.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"

BLE_Server BLES;

const char* BLE_NAME = "BC_LIGHT";
const char* SERVER_SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
const char* SERVER_CHAR_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8";

BLECharacteristic* sCharacteristic = NULL;

class MyCharCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* sCharacteristic) {
    std::string value = sCharacteristic->getValue();
    if (value.length() <= 0) {
      return;
    }

    String msg = "";
    for (int i = 0; i < value.length(); i++) {
      msg += value[i];
    }
#if DEBUG_LOG
    Serial.printf("[BLE Server] :: Receive Message : %s\n", msg.c_str());
#endif
  }
};

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* bServer){

  };
  void onDisconnect(BLEServer* bServer) {
    BLEDevice::stopAdvertising();
    vTaskDelay(BLES_AD_DELAY / portTICK_RATE_MS);
    BLEDevice::startAdvertising();
  }
};

BLE_Server::BLE_Server() {
}

void BLE_Server::start() {
  BLEDevice::init(BLE_NAME);

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


String BLE_Server::getAddress() {
  if (macAddress == "") {
    const uint8_t* point = esp_bt_dev_get_address();
    char addr[17];
    sprintf(addr, "%02X:%02X:%02X:%02X:%02X:%02X",
            (int)point[0], (int)point[1], (int)point[2],
            (int)point[3], (int)point[4], (int)point[5]);
    macAddress = String(addr);
  }
#if DEBUG_LOG
  Serial.printf("[BLE Server] :: Mac Address : %s\n", macAddress.c_str());
#endif
  return macAddress;
}