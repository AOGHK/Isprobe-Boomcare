#include "Thermometer.h"

Thermometer Thermo;

BLEUUID THERMO_SERVICE_UUID("00001809-0000-1000-8000-00805f9b34fb");
BLEUUID THERMO_CHAR_UUID("00002a1c-0000-1000-8000-00805f9b34fb");
BLEUUID CHAT_SERVICE_UUID("00002523-1212-efde-2523-785feabcd123");
BLEUUID CHAT_CHAR_UUID("00002525-1212-efde-2523-785feabcd123");

BLEScan* bleScan;
BLEClient* bleClient = nullptr;
BLEAdvertisedDevice* boomCareDevice = nullptr;
BLERemoteCharacteristic* chatCharacteristic = nullptr;

uint8_t boomcareSoundSta = 1;
uint8_t boomcareAddress[6] = { 0 };
static bool isBoomcareDiscovery = false;
static bool isBoomcareConnected = false;

xQueueHandle soundQueue = xQueueCreate(1, sizeof(uint8_t));

void syncMacAddress() {
  String _addr = String(boomCareDevice->getAddress().toString().c_str());
  const char* addrs = _addr.c_str();
  char* ptr;
  boomcareAddress[0] = strtol(addrs, &ptr, HEX);
  for (size_t idx = 1; idx < 6; idx++) {
    boomcareAddress[idx] = strtol(ptr + 1, &ptr, HEX);
  }
}

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(THERMO_SERVICE_UUID)) {
#if DEBUG_LOG
      Serial.printf("[Thermo] :: Boomcare Discovery.\n");
#endif
      BLEDevice::getScan()->stop();
      boomCareDevice = new BLEAdvertisedDevice(advertisedDevice);
      isBoomcareDiscovery = true;
    }
  }
};

void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic,
                    uint8_t* pData, size_t length, bool isNotify) {
  int16_t _thermo = (int16_t)((((uint16_t)pData[2]) << 8) + (uint16_t)pData[1]);
  Proc.sendEvtQueue(THERMO_MEASURE_RESULT, _thermo);
}


void syncSoundState() {
  if (chatCharacteristic != nullptr && chatCharacteristic->canRead()) {
    std::string value = chatCharacteristic->readValue();
    if (value.length() > 0) {
      boomcareSoundSta = (byte)value[0];
    }
  }
}

bool connect() {
  if (bleClient == nullptr) {
    bleClient = BLEDevice::createClient();
  }
  if (bleClient == nullptr || boomCareDevice == nullptr) {
    return false;
  }
  return bleClient->connect(boomCareDevice);
}

void discoverGatt() {
  bool res = bleClient->setMTU(517);
  if (!res) {
    bleClient->disconnect();
    return;
  }

  BLERemoteService* pRemoteService = bleClient->getService(THERMO_SERVICE_UUID);
  if (pRemoteService == nullptr) {
    bleClient->disconnect();
    return;
  }

  BLERemoteService* chatRemoteService = bleClient->getService(CHAT_SERVICE_UUID);
  if (chatRemoteService == nullptr) {
    bleClient->disconnect();
    return;
  }

  BLERemoteCharacteristic* remoteCharacteristic = pRemoteService->getCharacteristic(THERMO_CHAR_UUID);
  if (remoteCharacteristic == nullptr) {
    bleClient->disconnect();
    return;
  }

  chatCharacteristic = chatRemoteService->getCharacteristic(CHAT_CHAR_UUID);
  if (chatCharacteristic == nullptr) {
    bleClient->disconnect();
    return;
  }

  if (remoteCharacteristic->canIndicate()) {
    remoteCharacteristic->registerForNotify(notifyCallback, false);
  }

  syncSoundState();
  syncMacAddress();
}

void dispose() {
  // delete chatCharacteristic;
  // chatCharacteristic = nullptr;
  // delete boomCareDevice;
  // boomCareDevice = nullptr;
  delete bleClient;
  bleClient = nullptr;
  isBoomcareConnected = false;
  vTaskDelay(100 / portTICK_RATE_MS);
  BLEDevice::startAdvertising();
}

void taskCentralMode(void* param) {
  bleScan = BLEDevice::getScan();
  bleScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  bleScan->setInterval(100);
  bleScan->setWindow(99);
  bleScan->setActiveScan(true);  //active scan uses more power, but get results faster

  while (1) {
    uint8_t soundSta;
    if (xQueueReceive(soundQueue, &soundSta, 1 / portTICK_RATE_MS)) {
      if (isBoomcareConnected) {
        if (chatCharacteristic->canWrite()) {
          chatCharacteristic->writeValue(soundSta);
        }
        syncSoundState();
        Proc.sendEvtQueue(THERMO_GET_SOUND_STA, boomcareSoundSta);
      }
    }

    if (isBoomcareDiscovery) {
      isBoomcareConnected = connect();
      Proc.sendEvtQueue(THERMO_DISCOVERY, 0);
      if (isBoomcareConnected) {
        discoverGatt();
      } else {
        dispose();
      }
      Proc.sendEvtQueue(THERMO_CHANGE_CONNECT, isBoomcareConnected);
      isBoomcareDiscovery = false;
    } else if (isBoomcareConnected) {
      if (!bleClient->isConnected()) {
        dispose();
        Proc.sendEvtQueue(THERMO_CHANGE_CONNECT, false);
      }
    } else {
      bleScan->start(5, false);
      bleScan->clearResults();
      vTaskDelay(100 / portTICK_RATE_MS);
    }
    vTaskDelay(10 / portTICK_RATE_MS);
  }
}

Thermometer::Thermometer() {
}

void Thermometer::task() {
  xTaskCreate(taskCentralMode, "BLE_CENTRAL_TASK", 1024 * 8, NULL, 2, NULL);
}


bool Thermometer::isConnected() {
  return isBoomcareConnected;
}


uint8_t* Thermometer::getAddress() {
  return boomcareAddress;
}

uint8_t Thermometer::getSoundState() {
  return boomcareSoundSta;
}

void Thermometer::setSoundState(uint8_t _sta) {
  xQueueSend(soundQueue, (void*)&_sta, 1 / portTICK_RATE_MS);
}
