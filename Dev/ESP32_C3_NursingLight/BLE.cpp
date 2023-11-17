#include "BLE.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"

const char* BLE_NAME = "BC_LIGHT";
// extern xQueueHandle ble_queue;

//	## BLEClient variable
static BLEUUID BOOMCARE_SERVICE_UUID("00001809-0000-1000-8000-00805f9b34fb");
static BLEUUID BOOMCARE_CHAR_UUID("00002a1c-0000-1000-8000-00805f9b34fb");

static BLEClient* pClient;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* boomCareDevice;

bool isBoomcareDiscovery = false;
bool isBoomcareConnected = false;
int boomcareID = -1;

//  ## BLEServer variable
const char* SERVER_SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
const char* SERVER_CHAR_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8";

static BLEServer* pServer = NULL;
static BLEService* pService = NULL;
static BLECharacteristic* pCharacteristic = NULL;

String deviceMacAddress = "";

void (*_receiveCallback)(String);

void BLE::setBleReceiveCallback(void (*receiveCallback)(String)) {
  _receiveCallback = receiveCallback;
}

String BLE::getMacAddress() {
  if (deviceMacAddress == "") {
    const uint8_t* point = esp_bt_dev_get_address();
    char addr[17];
    sprintf(addr, "%02X:%02X:%02X:%02X:%02X:%02X",
            (int)point[0], (int)point[1], (int)point[2],
            (int)point[3], (int)point[4], (int)point[5]);
    deviceMacAddress = String(addr);
  }
  return deviceMacAddress;
}

void transferBleEvent(uint8_t _typeNum, String _recvMsg) {
  ble_evt_t evtData = {
    .typeNum = _typeNum,
    .recvStr = _recvMsg
  };
  xQueueSend(ble_queue, (void*)&evtData, 10 / portTICK_RATE_MS);
}

/* ===============================================
                BLE Client Funcs
=============================================== */
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(BOOMCARE_SERVICE_UUID)) {
      BLEDevice::getScan()->stop();
      boomCareDevice = new BLEAdvertisedDevice(advertisedDevice);
      isBoomcareDiscovery = true;
    }
  }
};

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
    boomcareID = pclient->getConnId();
  }

  void onDisconnect(BLEClient* pclient) {
    boomcareID = -1;
  }
};

static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  int16_t mValue = (int16_t)((((uint16_t)pData[2]) << 8) + (uint16_t)pData[1]);
  // #if DEBUG_LOG
  //   Serial.printf("(BLE Client) Measure Tmp Value : %d\n", mValue);
  // #endif
  transferBleEvent(BLE_MEASURE_TEMPERATURE, String(mValue));
}

bool connectToBoomcare() {
  pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(new MyClientCallback());
  pClient->connect(boomCareDevice);
  pClient->setMTU(517);

  BLERemoteService* pRemoteService = pClient->getService(BOOMCARE_SERVICE_UUID);
  if (pRemoteService == nullptr) {
    pClient->disconnect();
    return false;
  }

  pRemoteCharacteristic = pRemoteService->getCharacteristic(BOOMCARE_CHAR_UUID);
  if (pRemoteCharacteristic == nullptr) {
    pClient->disconnect();
    return false;
  }

  if (pRemoteCharacteristic->canRead()) {
    std::string value = pRemoteCharacteristic->readValue();
  }
  if (pRemoteCharacteristic->canIndicate()) {
    pRemoteCharacteristic->registerForNotify(notifyCallback, false);
  }
  return true;
}

void ble_client_Task(void* param) {
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);
  pBLEScan->setActiveScan(true);  //active scan uses more power, but get results faster

  while (1) {
    if (isBoomcareDiscovery) {
      long conn_start_time = millis();
      isBoomcareConnected = connectToBoomcare();
#if DEBUG_LOG
      Serial.printf("(BLE Client) Boomcare Connected : %d,\t Duration(ms) : %d\n", isBoomcareConnected, (int)(millis() - conn_start_time));
#endif
      if (isBoomcareConnected) {
        transferBleEvent(BLE_BOOMCARE_CONNECT, "");
      }
      isBoomcareDiscovery = false;
    } else if (isBoomcareConnected) {
      if (!pClient->isConnected()) {
        isBoomcareConnected = false;
        transferBleEvent(BLE_BOOMCARE_DISCONNECT, "");
        vTaskDelay(BLEC_AD_DELAY / portTICK_RATE_MS);
        BLEDevice::startAdvertising();
      }
    } else {
      BLEScanResults foundDevices = BLEDevice::getScan()->start(BOOMCARE_SCAN_TIME, false);
      BLEDevice::getScan()->clearResults();
      vTaskDelay(BLEC_SCAN_DELAY / portTICK_RATE_MS);
    }
    vTaskDelay(10 / portTICK_RATE_MS);
  }
}

/* ===============================================
                BLE Server Funcs
=============================================== */
class MyCharCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() > 0) {
      String data = "";
      for (int i = 0; i < value.length(); i++) {
        data += value[i];
      }
      // Recv callback (BLE_RECV_MESSAGE)
      if (_receiveCallback != nullptr && value[0] == '$' && value[value.length() - 1] == '#') {
        _receiveCallback(data);
      }
    }
  }
};

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* bServer, esp_ble_gatts_cb_param_t* param){

  };

  void onDisconnect(BLEServer* bServer) {
    if (boomcareID != bServer->getConnId()) {
      vTaskDelay(BLES_AD_DELAY / portTICK_RATE_MS);
      BLEDevice::startAdvertising();
    }
  }
};

void startServer() {
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());  // Server

  pService = pServer->createService(SERVER_SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(SERVER_CHAR_UUID, BLECharacteristic::PROPERTY_READ
                                                                       | BLECharacteristic::PROPERTY_WRITE
                                                                       | BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setCallbacks(new MyCharCallbacks());
  pService->start();

  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVER_SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x12);

  BLEDevice::startAdvertising();
}


/* ===============================================
                Main Funcs
=============================================== */
BLE::BLE() {
}

void BLE::init() {
  BLEDevice::init(BLE_NAME);
  xTaskCreatePinnedToCore(ble_client_Task, "BLE_CLIENT_TASK", 1024 * 4, NULL, 1, NULL, 0);
  startServer();
}

void BLE::writeStr(char header, char type, String data) {
  if (pCharacteristic == NULL) {
    return;
  }
  data = "$" + String(header) + String(type) + data + "#";
#if DEBUG_LOG
  Serial.printf("Transfer Str : %s\n", data);
#endif
  pCharacteristic->setValue(data.c_str());
  pCharacteristic->notify();
}
