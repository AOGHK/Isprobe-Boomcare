#include "BLE.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"

const char* BLE_NAME = "BC_LIGHT";
String MY_MAC_ADDRESS = "";
void (*_evtCallback)(ble_evt_t);

// ** BLE Peripheral
const char* SERVER_SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
const char* SERVER_CHAR_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8";

BLECharacteristic* sCharacteristic = NULL;

// ** BLE Central
BLEUUID THERMO_SERVICE_UUID("00001809-0000-1000-8000-00805f9b34fb");
BLEUUID THERMO_CHAR_UUID("00002a1c-0000-1000-8000-00805f9b34fb");
BLEUUID CHAT_SERVICE_UUID("00002523-1212-efde-2523-785feabcd123");
BLEUUID CHAT_CHAR_UUID("00002525-1212-efde-2523-785feabcd123");

BLEScan* bleScan;
BLEClient* bleClient = NULL;
BLEAdvertisedDevice* boomCareDevice = NULL;
BLERemoteCharacteristic* chatCharacteristic = NULL;

uint8_t boomcareSoundSta = 1;
String boomcareAddress = "";
int8_t boomcareID = -1;
static bool isBoomcareDiscovery = false;
static bool isBoomcareConnected = false;


unsigned long thermoConnTime = 0;
TaskHandle_t thermoConnHandle = NULL;

xQueueHandle blecQueue = xQueueCreate(5, sizeof(ble_evt_t));

#pragma region #__BLE Peripheral(Server) __ #
class MyCharCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* sCharacteristic) {
    std::string value = sCharacteristic->getValue();
    if (value.length() <= 0 || _evtCallback == nullptr) {
      return;
    }

    if (value[0] == '$' && value[value.length() - 1] == '#') {
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

void BLE::startPeripheralMode() {
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


#pragma region #__BLE Central(Client) __ #
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(THERMO_SERVICE_UUID)) {
#if DEBUG_LOG
      Serial.printf("[Central] :: Boomcare Discovery.\n");
#endif
      BLEDevice::getScan()->stop();
      boomCareDevice = new BLEAdvertisedDevice(advertisedDevice);
      if (_evtCallback != nullptr) {
        _evtCallback({
          ._type = BLEC_SCAN_DISCOVERY,
        });
      }
      thermoConnTime = millis();
      isBoomcareDiscovery = true;
    }
  }
};


void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic,
                    uint8_t* pData, size_t length, bool isNotify) {
  int16_t mValue = (int16_t)((((uint16_t)pData[2]) << 8) + (uint16_t)pData[1]);
  if (_evtCallback != nullptr) {
    _evtCallback({
      ._type = BLEC_RES_TEMPERATURE,
      ._num = mValue,
    });
  }
}

void resetThermoClient() {
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

void syncThermoSoundState() {
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

  syncThermoSoundState();
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
  if (_evtCallback != nullptr) {
    _evtCallback({
      ._type = BLEC_CHANGE_CONNECT,
      ._num = _isConn,
    });
  }
}

void taskCentralMode(void* param) {
  bleScan = BLEDevice::getScan();
  bleScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  bleScan->setInterval(100);
  bleScan->setWindow(99);
  bleScan->setActiveScan(true);  //active scan uses more power, but get results faster

  while (1) {
    ble_evt_t evt;
    if (xQueueReceive(blecQueue, &evt, 10 / portTICK_RATE_MS)) {
      if (evt._type == BLEC_CHANGE_SOUND_STA) {
        if (isBoomcareConnected) {
          if (chatCharacteristic->canWrite()) {
            chatCharacteristic->writeValue(evt._num);
          }
          syncThermoSoundState();
        }
      }
    }

    if (isBoomcareDiscovery) {
      bool _isConn = connectBoomcare();
      submitBoomcareConnected(_isConn);
      if (_isConn) {
        isBoomcareConnected = setGattBoomcare();
        submitBoomcareConnected(isBoomcareConnected);
      }
      if (!_isConn || !isBoomcareConnected) {
        resetThermoClient();
      }
      isBoomcareDiscovery = false;
    } else if (isBoomcareConnected) {
      if (!bleClient->isConnected()) {
        isBoomcareConnected = false;
        submitBoomcareConnected(isBoomcareConnected);
        resetThermoClient();
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

BLE::BLE() {
}

void BLE::begin() {
  BLEDevice::init(BLE_NAME);

  const uint8_t* point = esp_bt_dev_get_address();
  char addr[17];
  sprintf(addr, "%02X:%02X:%02X:%02X:%02X:%02X",
          (int)point[0], (int)point[1], (int)point[2],
          (int)point[3], (int)point[4], (int)point[5]);
  MY_MAC_ADDRESS = String(addr);
#if DEBUG_LOG
  Serial.printf("[BLE] :: My Address : %s\n", MY_MAC_ADDRESS.c_str());
#endif

  startPeripheralMode();
  // xTaskCreatePinnedToCore(taskCentralMode, "BLE_CENTRAL_TASK", 1024 * 16, NULL, 3, NULL, 0);
  xTaskCreate(taskCentralMode, "BLE_CENTRAL_TASK", 1024 * 8, NULL, 2, NULL);
}

void BLE::setCallback(void (*evtCallback)(ble_evt_t)) {
  _evtCallback = evtCallback;
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

void BLE::transferSoundState() {
  if (sCharacteristic == NULL) {
    return;
  }
  String data = "$3C" + String(boomcareSoundSta) + "#";
  sCharacteristic->setValue(data.c_str());
  sCharacteristic->notify();
}

void BLE::transferMyAddress() {
  if (sCharacteristic == NULL) {
    return;
  }
  String data = "$4" + boomcareAddress + "#";
  sCharacteristic->setValue(data.c_str());
  sCharacteristic->notify();
}

bool BLE::isSameThermo(String _address) {
  return _address.equals(boomcareAddress);
}
