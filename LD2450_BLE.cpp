#include "LD2450_BLE.h"

BLEClient* LD2450_BLE::pClient = nullptr;
BLERemoteCharacteristic* LD2450_BLE::pRemoteCharacteristic = nullptr;
std::vector<uint8_t> LD2450_BLE::dataBuffer;
bool LD2450_BLE::connected = false;
bool LD2450_BLE::newDataAvailable = false;
BLEAddress LD2450_BLE::sensorAddress("");
int16_t LD2450_BLE::offsetX = 0;
int16_t LD2450_BLE::offsetY = 0;
float LD2450_BLE::rotationDegrees = 0.0;
std::map<std::string, int> LD2450_BLE::scanResults;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) override {
        if (advertisedDevice.haveName()) {
            std::string deviceName = advertisedDevice.getName();
            if (deviceName.find("HLK-LD2450") != std::string::npos) {
                Serial.print("Found Device: ");
                Serial.print(advertisedDevice.getAddress().toString().c_str());
                Serial.print(", RSSI: ");
                Serial.println(advertisedDevice.getRSSI());

                LD2450_BLE::addScanResult(advertisedDevice.getAddress().toString(), advertisedDevice.getRSSI());
            }
        }
    }
};

void LD2450_BLE::init(const char* address) {
    sensorAddress = BLEAddress(address);
    BLEDevice::init("");
}

bool LD2450_BLE::isConnected() {
    return connected && (pClient != nullptr) && pClient->isConnected();
}

void LD2450_BLE::connect() {
    if (pClient == nullptr) {
        pClient = BLEDevice::createClient();
    }

    connected = false;
    if (pClient->connect(sensorAddress)) {
        BLERemoteService* pRemoteService = pClient->getService(BLEUUID("0000fff0-0000-1000-8000-00805f9b34fb"));
        if (pRemoteService != nullptr) {
            pRemoteCharacteristic = pRemoteService->getCharacteristic(BLEUUID("0000fff1-0000-1000-8000-00805f9b34fb"));
            if (pRemoteCharacteristic != nullptr && pRemoteCharacteristic->canNotify()) {
                pRemoteCharacteristic->registerForNotify(notifyCallback);
                connected = true;
            }
        }
    } else {
        connected = false;
    }
}

void LD2450_BLE::attemptReconnect() {
    if (!isConnected()) {
        connect();
    }
}

void LD2450_BLE::setSensorOffset(int16_t x, int16_t y) {
    offsetX = x;
    offsetY = y;
}

void LD2450_BLE::setSensorRotation(float degrees) {
    rotationDegrees = degrees;
}

std::vector<uint8_t> LD2450_BLE::rawData() {
    processBuffer();
    std::vector<uint8_t> tempData;
    if (newDataAvailable && dataBuffer.size() >= 30) {
        tempData.insert(tempData.end(), dataBuffer.begin(), dataBuffer.begin() + 30);
        dataBuffer.erase(dataBuffer.begin(), dataBuffer.begin() + 30);
        newDataAvailable = false;
    }
    return tempData;
}

void LD2450_BLE::notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
    for (size_t i = 0; i < length; i++) {
        dataBuffer.push_back(pData[i]);
    }
}

void LD2450_BLE::processBuffer() {
    size_t start = 0;
    while (start + 30 <= dataBuffer.size()) {
        if (dataBuffer[start] == 0xAA && dataBuffer[start + 1] == 0xFF &&
            dataBuffer[start + 2] == 0x03 && dataBuffer[start + 3] == 0x00 &&
            dataBuffer[start + 29] == 0xCC && dataBuffer[start + 28] == 0x55) {
            break;
        }
        start++;
    }

    if (start + 30 <= dataBuffer.size()) {
        std::rotate(dataBuffer.begin(), dataBuffer.begin() + start, dataBuffer.end());
        newDataAvailable = true;
    }
}

std::vector<Target> LD2450_BLE::getTargets() {
    std::vector<Target> targets;
    auto frame = rawData();
    if (!frame.empty()) {
        for (int i = 0; i < 3; i++) {
            Target target;
            target.x = (frame[i * 8 + 5] << 8) | frame[i * 8 + 4];
            if (frame[i * 8 + 5] & 0x80) target.x = -(target.x - 0x8000);
            target.y = (frame[i * 8 + 7] << 8) | frame[i * 8 + 6];
            if (frame[i * 8 + 7] & 0x80) target.y = -(target.y - 0x8000);
            target.speed = (frame[i * 8 + 9] << 8) | frame[i * 8 + 8];
            if ((frame[i * 8 + 9] & 0x80) != 0) target.speed = -(target.speed - 0x8000);
            
            target.active = (target.x != 0 || target.y != 0);
            if (target.active) {
                target = transformTarget(target);
                target.radius = sqrt(target.x * target.x + target.y * target.y);
                target.angle = atan2(target.y, target.x) * 180.0 / M_PI;
            } else {
                target.radius = 0;
                target.angle = 0;
            }
            targets.push_back(target);
        }
    }
    return targets;
}

Target LD2450_BLE::transformTarget(const Target& originalTarget) {
    Target transformed = originalTarget;
    float rad = rotationDegrees * M_PI / 180.0;
    transformed.x = cos(rad) * originalTarget.x - sin(rad) * originalTarget.y + offsetX;
    transformed.y = sin(rad) * originalTarget.x + cos(rad) * originalTarget.y + offsetY;
    return transformed;
}

void LD2450_BLE::findLD2450() {
    BLEDevice::init(""); // Stelle sicher, dass dies aufgerufen wird
    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true); // Aktiver Scan
    pBLEScan->start(10); // Scanne für 10 Sekunden
}

void LD2450_BLE::addScanResult(const std::string& address, int rssi) {
    scanResults[address] = rssi;
}

String LD2450_BLE::getClosestSensor() {
    std::pair<std::string, int> closestSensor;
    for (const auto& result : scanResults) {
        if (closestSensor.first.empty() || result.second > closestSensor.second) {
            closestSensor = result;
        }
    }
    return String(closestSensor.first.c_str());
}
