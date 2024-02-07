#include "Thermo.h"

Thermometer Thermo;

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
// int8_t boomcareID = -1;
static bool isBoomcareDiscovery = false;
static bool isBoomcareConnected = false;

xQueueHandle soundQueue = xQueueCreate(1, sizeof(bool));
xQueueHandle thermoQueue = xQueueCreate(1, sizeof(thermo_evt_t));

void sendThermoQueue(uint8_t _type, uint16_t _data) {
  thermo_evt_t evt = {
    .type = _type,
    .result = _data,
  };
  xQueueSend(thermoQueue, (void*)&evt, 5 / portTICK_RATE_MS);
}

void disposeBoomcare() {
  delete bleClient;
  bleClient = NULL;
  boomcareAddress = "";
  // boomcareID = -1;
  isBoomcareConnected = false;
  vTaskDelay(BLES_AD_DELAY / portTICK_RATE_MS);
  BLEDevice::startAdvertising();
}

void syncBoomcareSoundState() {
  if (chatCharacteristic != NULL && chatCharacteristic->canRead()) {
    std::string value = chatCharacteristic->readValue();
    if (value.length() > 0) {
      boomcareSoundSta = (byte)value[0];
#if DEBUG_LOG
      Serial.printf("[Thermo] :: Boomcare Sound Sta : %d\n", boomcareSoundSta);
#endif
    }
  }
}

void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic,
                    uint8_t* pData, size_t length, bool isNotify) {
  int16_t _thermo = (int16_t)((((uint16_t)pData[2]) << 8) + (uint16_t)pData[1]);
  sendThermoQueue(THERMO_MEASURE_RESULT, _thermo);
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

void discoverGatt() {
  bool res = bleClient->setMTU(517);
  if (!res) {
    bleClient->disconnect();
    return;
  }

  BLERemoteService* pRemoteService = bleClient->getService(THERMO_SERVICE_UUID);
  if (pRemoteService == NULL) {
    bleClient->disconnect();
    return;
  }

  BLERemoteService* chatRemoteService = bleClient->getService(CHAT_SERVICE_UUID);
  if (chatRemoteService == NULL) {
    bleClient->disconnect();
    return;
  }

  BLERemoteCharacteristic* remoteCharacteristic = pRemoteService->getCharacteristic(THERMO_CHAR_UUID);
  if (remoteCharacteristic == NULL) {
    bleClient->disconnect();
    return;
  }

  chatCharacteristic = chatRemoteService->getCharacteristic(CHAT_CHAR_UUID);
  if (chatCharacteristic == NULL) {
    bleClient->disconnect();
    return;
  }

  // if (remoteCharacteristic->canRead()) {
  //   std::string value = remoteCharacteristic->readValue();
  // }

  if (remoteCharacteristic->canIndicate()) {
    remoteCharacteristic->registerForNotify(notifyCallback, false);
  }

  syncBoomcareSoundState();
  // boomcareID = bleClient->getConnId();
  boomcareAddress = String(boomCareDevice->getAddress().toString().c_str());
#if DEBUG_LOG
  Serial.printf("[Thermo] :: Boomcare Address : %s\n", boomcareAddress.c_str());
#endif
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

void taskCentralMode(void* param) {
  bleScan = BLEDevice::getScan();
  bleScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  bleScan->setInterval(100);
  bleScan->setWindow(99);
  bleScan->setActiveScan(true);  //active scan uses more power, but get results faster

  while (1) {
    bool soundSta;
    if (xQueueReceive(soundQueue, &soundSta, 10 / portTICK_RATE_MS)) {
      if (isBoomcareConnected) {
        if (chatCharacteristic->canWrite()) {
          chatCharacteristic->writeValue(soundSta);
        }
        syncBoomcareSoundState();
        sendThermoQueue(THERMO_GET_SOUND_STA, boomcareSoundSta);
      }
    }

    if (isBoomcareDiscovery) {
      isBoomcareConnected = connectBoomcare();
      sendThermoQueue(THERMO_CHANGE_CONNECT, isBoomcareConnected);
#if DEBUG_LOG
      Serial.printf("[Thermo] :: Boomcare connected - %d\n", isBoomcareConnected);
#endif
      if (isBoomcareConnected) {
        discoverGatt();
      } else {
        disposeBoomcare();
      }
      isBoomcareDiscovery = false;
    } else if (isBoomcareConnected) {
      if (!bleClient->isConnected()) {
#if DEBUG_LOG
        Serial.println("[Thermo] :: Boomcare disconnected.");
#endif
        disposeBoomcare();
        sendThermoQueue(THERMO_CHANGE_CONNECT, false);
      }
    } else {
      bleScan->start(BLEC_SCAN_SEC, false);
      bleScan->clearResults();
      vTaskDelay(BLEC_SCAN_DELAY / portTICK_RATE_MS);
    }
    vTaskDelay(10 / portTICK_RATE_MS);
  }
}


Thermometer::Thermometer() {
}

bool Thermometer::isConnected() {
  return isBoomcareConnected;
}

bool Thermometer::isSameDevice(String _address) {
  return _address.equals(boomcareAddress);
}

void Thermometer::task() {
  BLEDevice::init("BC-Light");

  xTaskCreate(taskCentralMode, "BLE_CENTRAL_TASK", 1024 * 8, NULL, 2, NULL);
}

uint8_t Thermometer::getSoundState() {
  return boomcareSoundSta;
}

uint8_t Thermometer::setSoundState(uint8_t _sta) {
  xQueueSend(soundQueue, (void*)&_sta, 10 / portTICK_RATE_MS);
}