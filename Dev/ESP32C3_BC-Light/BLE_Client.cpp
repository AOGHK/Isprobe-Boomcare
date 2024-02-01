#include "BLE_Client.h"

BLE_Client BLEC;

BLEUUID THERMOMETER_SERVICE_UUID("00001809-0000-1000-8000-00805f9b34fb");
BLEUUID THERMOMETER_CHAR_UUID("00002a1c-0000-1000-8000-00805f9b34fb");
BLEUUID CHAT_SERVICE_UUID("00002523-1212-efde-2523-785feabcd123");
BLEUUID CHAT_CHAR_UUID("00002525-1212-efde-2523-785feabcd123");

BLEScan* bleScan;
BLEClient* bleClient = NULL;
BLEAdvertisedDevice* boomCareDevice = NULL;
BLERemoteCharacteristic* chatCharacteristic = NULL;

xQueueHandle soundQueue = xQueueCreate(1, sizeof(bool));

String boomcareAddress = "";
int8_t boomcareID = -1;

uint8_t isSoundEnable = 1;
bool isDiscovery = false;
bool isConnected = false;


class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(THERMOMETER_SERVICE_UUID)) {
#if DEBUG_LOG
      Serial.printf("[BLE Client] :: Boomcare Discovery.\n");
#endif
      BLEDevice::getScan()->stop();
      boomCareDevice = new BLEAdvertisedDevice(advertisedDevice);
      // + Evt -> BLEC_SCAN_DISCOVERY
      isDiscovery = true;
    }
  }
};

void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic,
                    uint8_t* pData, size_t length, bool isNotify) {
  int16_t temperatureValue = (int16_t)((((uint16_t)pData[2]) << 8) + (uint16_t)pData[1]);
#if DEBUG_LOG
  Serial.printf("[BLE Client] :: Temperature Value : %d\n", temperatureValue);
#endif
  // + Evt -> BLEC_RES_TEMPERATURE
}

void dispose() {
  if (bleClient->isConnected()) {
    bleClient->disconnect();
  }
  delete bleClient;
  bleClient = NULL;
  boomcareAddress = "";
  boomcareID = -1;
  vTaskDelay(BLES_AD_DELAY / portTICK_RATE_MS);
  BLEDevice::startAdvertising();
}

void readSoundState() {
  if (chatCharacteristic != NULL && chatCharacteristic->canRead()) {
    std::string value = chatCharacteristic->readValue();
    if (value.length() > 0) {
      isSoundEnable = (byte)value[0];
#if DEBUG_LOG
      Serial.printf("[BLE Client] :: Boomcare Sound State : %d\n", isSoundEnable);
#endif
    }
  }
}

void writeSoundState(uint8_t _enable) {
  if (!isConnected) {
    return;
  }
  if (chatCharacteristic->canWrite()) {
    chatCharacteristic->writeValue(_enable);
  }
  readSoundState();
}

bool connectDevice() {
  if (bleClient == NULL) {
    bleClient = BLEDevice::createClient();
  }
  if (bleClient == nullptr || boomCareDevice == nullptr) {
    return false;
  }
  return bleClient->connect(boomCareDevice);
}

bool discoveryService() {
  bool res = bleClient->setMTU(517);
  if (!res) {
    bleClient->disconnect();
    return false;
  }

  BLERemoteService* pRemoteService = bleClient->getService(THERMOMETER_SERVICE_UUID);
  if (pRemoteService == NULL) {
    bleClient->disconnect();
    return false;
  }

  BLERemoteService* chatRemoteService = bleClient->getService(CHAT_SERVICE_UUID);
  if (chatRemoteService == NULL) {
    bleClient->disconnect();
    return false;
  }

  BLERemoteCharacteristic* remoteCharacteristic = pRemoteService->getCharacteristic(THERMOMETER_CHAR_UUID);
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

  readSoundState();
  boomcareID = bleClient->getConnId();
  boomcareAddress = String(boomCareDevice->getAddress().toString().c_str());
#if DEBUG_LOG
  Serial.printf("[BLE Client] :: Boomcare Address : %s, Conn Id : %d\n", boomcareAddress.c_str(), boomcareID);
#endif
  return true;
}

void transferConnection(bool _isConn) {
  isConnected = _isConn;
#if DEBUG_LOG
  Serial.printf("[BLE Client] :: Boomcare Connected : %d\n", _isConn);
#endif
  // + Evt -> BLEC_CHANGE_CONNECT
}

void taskClient(void* param) {
  bleScan = BLEDevice::getScan();
  bleScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  bleScan->setInterval(100);
  bleScan->setWindow(99);
  bleScan->setActiveScan(true);

  while (1) {
    uint8_t _sta;
    if (xQueueReceive(soundQueue, &_sta, 10 / portTICK_RATE_MS)) {
      writeSoundState(_sta);
    }

    if (isDiscovery) {
      bool _isConnected = connectDevice();
      transferConnection(_isConnected);
      if (_isConnected) {
        discoveryService();
      }
      isDiscovery = false;
    } else if (isConnected) {
      if (!bleClient->isConnected()) {
        transferConnection(false);
        dispose();
      }
    } else {
      bleScan->start(BLEC_SCAN_SEC, false);
      bleScan->clearResults();
      vTaskDelay(BLEC_SCAN_DELAY / portTICK_RATE_MS);
    }
    vTaskDelay(10 / portTICK_RATE_MS);
  }
}


BLE_Client::BLE_Client() {
}

void BLE_Client::start() {
  xTaskCreate(taskClient, "BLE_CENTRAL_TASK", 1024 * 8, NULL, 2, NULL);
}

void BLE_Client::setSound(bool _enable) {
  xQueueSend(soundQueue, (void*)&_enable, 10 / portTICK_RATE_MS);
}