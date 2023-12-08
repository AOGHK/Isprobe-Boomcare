#include "BLE.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"

const char* BLE_NAME = "BC_LIGHT";
void (*_evtCallback)(ble_evt_t);

BLE::BLE() {
}

void BLE::setEvnetCallback(void (*evtCallback)(ble_evt_t)) {
  _evtCallback = evtCallback;
}

#pragma region BLE Server - App Connect& Set Config
static BLEUUID THERMO_SERVICE_UUID("00001809-0000-1000-8000-00805f9b34fb");
static BLEUUID THERMO_CHAR_UUID("00002a1c-0000-1000-8000-00805f9b34fb");

static BLEUUID CHAT_SERVICE_UUID("00002523-1212-efde-2523-785feabcd123");
static BLEUUID CHAT_CHAR_UUID("00002525-1212-efde-2523-785feabcd123");

static BLEClient* bleClient;
static BLEAdvertisedDevice* boomCareDevice;
String boomcareAddress = "";

BLERemoteCharacteristic* chatCharacteristic;

static bool isBoomcareDiscovery = false;
static bool isBoomcareConnected = false;
int boomcareID = -1;

xQueueHandle bcQueue = xQueueCreate(2, sizeof(ble_evt_t));

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(THERMO_SERVICE_UUID)) {
      BLEDevice::getScan()->stop();
      boomCareDevice = new BLEAdvertisedDevice(advertisedDevice);
      ble_evt_t evtData = {
        ._type = BLEC_SCAN_DISCOVERY,
      };
      if (_evtCallback != nullptr) {
        _evtCallback(evtData);
      }
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
  if (_evtCallback != nullptr) {
    ble_evt_t evtData = {
      ._type = BLEC_RES_TEMPERATURE,
      ._num = mValue
    };
    _evtCallback(evtData);
  }
}

void readBoomcareSoundState() {
  if (chatCharacteristic->canRead()) {
    std::string value = chatCharacteristic->readValue();
    if (value.length() > 0) {
      uint8_t sta = (byte)value[0];
      ble_evt_t evtData = {
        ._type = BLEC_RES_STA_SOUND,
        ._num = sta
      };
      _evtCallback(evtData);
    }
  }
}

static bool connectBoomcare() {
  bleClient = BLEDevice::createClient();
  bleClient->setClientCallbacks(new MyClientCallback());
  return bleClient->connect(boomCareDevice);
}

static bool setGattBoomcare() {
  bool res = bleClient->setMTU(517);
#ifdef DEBUG_LOG
  Serial.printf("BLE Client Set MTU : %d\n", res);
#endif
  if (!res) {
    bleClient->disconnect();
    return false;
  }

  BLERemoteService* pRemoteService = bleClient->getService(THERMO_SERVICE_UUID);
#ifdef DEBUG_LOG
  Serial.println("BLE Get Service -> THERMO_SERVICE_UUID");
#endif
  if (pRemoteService == nullptr) {
    bleClient->disconnect();
    return false;
  }

  BLERemoteService* chatRemoteService = bleClient->getService(CHAT_SERVICE_UUID);
#ifdef DEBUG_LOG
  Serial.println("BLE Get Service -> CHAT_SERVICE_UUID");
#endif
  if (chatRemoteService == nullptr) {
    bleClient->disconnect();
    return false;
  }

  BLERemoteCharacteristic* remoteCharacteristic = pRemoteService->getCharacteristic(THERMO_CHAR_UUID);
#ifdef DEBUG_LOG
  Serial.println("BLE Get Characteristic -> THERMO_CHAR_UUID");
#endif
  if (remoteCharacteristic == nullptr) {
    bleClient->disconnect();
    return false;
  }

  chatCharacteristic = chatRemoteService->getCharacteristic(CHAT_CHAR_UUID);
#ifdef DEBUG_LOG
  Serial.println("BLE Get Characteristic -> CHAT_CHAR_UUID");
#endif
  if (chatCharacteristic == nullptr) {
    bleClient->disconnect();
    return false;
  }
  readBoomcareSoundState();
  if (remoteCharacteristic->canRead()) {
    std::string value = remoteCharacteristic->readValue();
  }
  if (remoteCharacteristic->canIndicate()) {
    remoteCharacteristic->registerForNotify(notifyCallback, false);
  }
  return true;
}

void submitBoomcareState(bool _isConn) {
  ble_evt_t evtData = {
    ._type = BLEC_CHANGE_CONNECT,
    ._num = _isConn
  };
  if (_evtCallback != nullptr) {
    _evtCallback(evtData);
  }
}

void taskBleClient(void* param) {
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);
  pBLEScan->setActiveScan(true);  //active scan uses more power, but get results faster

  while (1) {
    ble_evt_t recvData;
    if (xQueueReceive(bcQueue, &recvData, 10 / portTICK_RATE_MS)) {
      if (recvData._type == BLEC_CHANGE_SOUND_MODE) {
        if (isBoomcareConnected) {
          if (chatCharacteristic->canWrite()) {
            chatCharacteristic->writeValue(recvData._num);
          }
          readBoomcareSoundState();
        }
      }
    }

    if (isBoomcareDiscovery) {
      bool _isConnect = connectBoomcare();
      submitBoomcareState(_isConnect);
      if (_isConnect) {
        isBoomcareConnected = setGattBoomcare();
        if (isBoomcareConnected) {
          boomcareAddress = String(boomCareDevice->getAddress().toString().c_str());
        } else {
          delete bleClient;
          submitBoomcareState(isBoomcareConnected);
        }
      }
      isBoomcareDiscovery = false;
    } else if (isBoomcareConnected) {
      if (!bleClient->isConnected()) {
        boomcareAddress = "";
        isBoomcareConnected = false;
        submitBoomcareState(isBoomcareConnected);
        vTaskDelay(BLEC_AD_DELAY / portTICK_RATE_MS);
        BLEDevice::startAdvertising();
      }
    } else {
      BLEScanResults foundDevices = BLEDevice::getScan()->start(BOOMCARE_SCAN_SEC, false);
      BLEDevice::getScan()->clearResults();
      vTaskDelay(BLEC_SCAN_DELAY / portTICK_RATE_MS);
    }
    vTaskDelay(10 / portTICK_RATE_MS);
  }
}
#pragma endregion

#pragma region BLE Server - App Connect& Set Config
const char* SERVER_SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
const char* SERVER_CHAR_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8";

class MyCharCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* sCharacteristic) {
    std::string value = sCharacteristic->getValue();
    if (value.length() > 0) {
      if (_evtCallback != nullptr && value[0] == '$' && value[value.length() - 1] == '#') {
        ble_evt_t evtData = {};
        evtData._str = "";
        for (int i = 2; i < value.length() - 1; i++) {
          evtData._str += value[i];
        }
        if (value[1] == 0x31) {  // ## Ctrl
          evtData._type = BLES_RECV_CTRL_DATA;
        } else if (value[1] == 0x32) {  // ## Setup
          evtData._type = BLES_RECV_SETUP_DATA;
        } else if (value[1] == 0x33) {  // ## Check
          evtData._type = BLES_RECV_REQ_DATA;
        } else if (value[1] == 0x34) {  // ## Req Address
          evtData._type = BLES_RECV_REQ_ADDRESS;
        }
        _evtCallback(evtData);
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

void BLE::startServer() {
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

void BLE::begin() {
  BLEDevice::init(BLE_NAME);
  startServer();
  xTaskCreatePinnedToCore(taskBleClient, "BLE_CLIENT_TASK", 1024 * 8, NULL, 1, NULL, 0);
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

String BLE::getBoomcareAddress() {
  return boomcareAddress;
}

void BLE::writeData(char header, char type, String data) {
  if (sCharacteristic == NULL) {
    return;
  }
  data = "$" + String(header) + String(type) + data + "#";
  sCharacteristic->setValue(data.c_str());
  sCharacteristic->notify();
}

void BLE::writeData(char header, String data) {
  if (sCharacteristic == NULL) {
    return;
  }
  data = "$" + String(header) + data + "#";
  sCharacteristic->setValue(data.c_str());
  sCharacteristic->notify();
}
