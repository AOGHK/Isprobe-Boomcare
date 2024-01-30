#include "BLE.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"

BLEClass BLE;

const char* BLE_NAME = "BC_LIGHT";

// # BLE Peripheral
const char* SERVER_SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
const char* SERVER_CHAR_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8";

BLECharacteristic* sCharacteristic = NULL;

// # BLE Central
BLEUUID THERMO_SERVICE_UUID("00001809-0000-1000-8000-00805f9b34fb");
BLEUUID THERMO_CHAR_UUID("00002a1c-0000-1000-8000-00805f9b34fb");
BLEUUID CHAT_SERVICE_UUID("00002523-1212-efde-2523-785feabcd123");
BLEUUID CHAT_CHAR_UUID("00002525-1212-efde-2523-785feabcd123");

BLEScan* bleScan;
BLEClient* bleClient = NULL;
BLEAdvertisedDevice* boomCareDevice = NULL;
BLERemoteCharacteristic* chatCharacteristic = NULL;
xQueueHandle blecQueue = xQueueCreate(5, sizeof(ble_evt_t));
uint8_t boomcareSoundSta = 1;
String boomcareAddress = "";
int8_t boomcareID = -1;
static bool isBoomcareDiscovery = false;
static bool isBoomcareConnected = false;

#pragma region>> BLE Peripheral(Server)
class MyCharCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* sCharacteristic) {
    std::string value = sCharacteristic->getValue();
    if (value.length() <= 0) {
      return;
    }
    if (value[0] == '$' && value[value.length() - 1] == '#') {
      // + AT Command
    }
  }
};

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* bServer){

  };
  void onDisconnect(BLEServer* bServer) {
    if (boomcareID != bServer->getConnId()) {
      vTaskDelay(BLES_AD_DELAY / portTICK_RATE_MS);
      BLEDevice::startAdvertising();
    }
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
#pragma endregion



#pragma region>> BLE Central(Client)
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(THERMO_SERVICE_UUID)) {
#if DEBUG_LOG
      Serial.printf("[Central] :: Boomcare Discovery.\n");
#endif
      BLEDevice::getScan()->stop();
      boomCareDevice = new BLEAdvertisedDevice(advertisedDevice);
      // + Evt -> BLEC_SCAN_DISCOVERY
      isBoomcareDiscovery = true;
    }
  }
};


void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic,
                    uint8_t* pData, size_t length, bool isNotify) {
  int16_t mValue = (int16_t)((((uint16_t)pData[2]) << 8) + (uint16_t)pData[1]);
  // + Evt -> BLEC_RES_TEMPERATURE
}

void disposeBoomcareClient() {
  if (bleClient->isConnected()) {
    bleClient->disconnect();
  }
  delete bleClient;
  bleClient = NULL;
  boomcareAddress = "";
  boomcareID = -1;
  vTaskDelay(BLEC_AD_DELAY / portTICK_RATE_MS);
  BLEDevice::startAdvertising();
}

void getBoomcareSoundState() {
  if (chatCharacteristic != NULL && chatCharacteristic->canRead()) {
    std::string value = chatCharacteristic->readValue();
    if (value.length() > 0) {
      boomcareSoundSta = (byte)value[0];
#if DEBUG_LOG
      Serial.printf("[Central] :: Boomcare Sound Sta : %d\n", boomcareSoundSta);
#endif
    }
  }
}

bool connectBoomcare() {
  if (bleClient == NULL) {
    bleClient = BLEDevice::createClient();
  }
  if (bleClient == nullptr || boomCareDevice == nullptr) {
    return false;
  }
  return bleClient->connect(boomCareDevice);
}

bool setGattBoomcare() {
  bool res = bleClient->setMTU(517);
  if (!res) {
    bleClient->disconnect();
    return false;
  }

  BLERemoteService* pRemoteService = bleClient->getService(THERMO_SERVICE_UUID);
  if (pRemoteService == NULL) {
    bleClient->disconnect();
    return false;
  }

  BLERemoteService* chatRemoteService = bleClient->getService(CHAT_SERVICE_UUID);
  if (chatRemoteService == NULL) {
    bleClient->disconnect();
    return false;
  }

  BLERemoteCharacteristic* remoteCharacteristic = pRemoteService->getCharacteristic(THERMO_CHAR_UUID);
  if (remoteCharacteristic == NULL) {
    bleClient->disconnect();
    return false;
  }

  chatCharacteristic = chatRemoteService->getCharacteristic(CHAT_CHAR_UUID);
  if (chatCharacteristic == NULL) {
    bleClient->disconnect();
    return false;
  }

  if (remoteCharacteristic->canRead()) {
    std::string value = remoteCharacteristic->readValue();
  }

  if (remoteCharacteristic->canIndicate()) {
    remoteCharacteristic->registerForNotify(notifyCallback, false);
  }

  getBoomcareSoundState();
  boomcareID = bleClient->getConnId();
  boomcareAddress = String(boomCareDevice->getAddress().toString().c_str());
#if DEBUG_LOG
  Serial.printf("[Central] :: Boomcare Address : %s, Conn Id : %d\n", boomcareAddress.c_str(), boomcareID);
#endif
  return true;
}

void submitBoomcareConnected(bool _isConn) {
#if DEBUG_LOG
  Serial.printf("[Central] :: Boomcare Connected : %d\n", _isConn);
#endif
  // + Evt -> BLEC_CHANGE_CONNECT
}

void taskCentralMode(void* param) {
  bleScan = BLEDevice::getScan();
  bleScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  bleScan->setInterval(100);
  bleScan->setWindow(99);
  bleScan->setActiveScan(true);

  while (1) {
    ble_evt_t evt;
    if (xQueueReceive(blecQueue, &evt, 10 / portTICK_RATE_MS)) {
    }

    if (isBoomcareDiscovery) {
      bool _isConn = connectBoomcare();
      submitBoomcareConnected(_isConn);
      if (_isConn) {
        isBoomcareConnected = setGattBoomcare();
        submitBoomcareConnected(isBoomcareConnected);
      }
      if (!_isConn || !isBoomcareConnected) {
        disposeBoomcareClient();
      }
      isBoomcareDiscovery = false;
    } else if (isBoomcareConnected) {
      if (!bleClient->isConnected()) {
        isBoomcareConnected = false;
        submitBoomcareConnected(isBoomcareConnected);
        disposeBoomcareClient();
      }
    } else {
      bleScan->start(BLEC_SCAN_SEC, false);
      bleScan->clearResults();
      vTaskDelay(BLEC_SCAN_DELAY / portTICK_RATE_MS);
    }
    vTaskDelay(10 / portTICK_RATE_MS);
  }
}
#pragma endregion


BLEClass::BLEClass() {
}

void BLEClass::begin() {
  BLEDevice::init(BLE_NAME);
  getMyAddress();
  startPeripheralMode();
  xTaskCreate(taskCentralMode, "BLE_CENTRAL_TASK", 1024 * 8, NULL, 2, NULL);
}

String BLEClass::getMyAddress() {
  if (myMacAddress == "") {
    const uint8_t* point = esp_bt_dev_get_address();
    char addr[17];
    sprintf(addr, "%02X:%02X:%02X:%02X:%02X:%02X",
            (int)point[0], (int)point[1], (int)point[2],
            (int)point[3], (int)point[4], (int)point[5]);
    myMacAddress = String(addr);
  }
#if DEBUG_LOG
  Serial.printf("[BLE] :: My Address : %s\n", myMacAddress.c_str());
#endif
  return myMacAddress;
}

bool BLEClass::isSameBoomcare(String _address) {
  return _address.equals(boomcareAddress);
}