#ifndef LD2450_BLE_h
#define LD2450_BLE_h

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <vector>
#include <map>
#include <math.h>

struct Target {
    int16_t x; // X coordinate
    int16_t y; // Y coordinate
    int16_t speed; // Speed
    float radius; // Polar coordinate - radius
    float angle; // Polar coordinate - angle in degrees
    bool active; // Indicates if the target is active
};

class LD2450_BLE {
public:
    void init(const char* address);
    void setSensorOffset(int16_t offsetX, int16_t offsetY);
    void setSensorRotation(float degrees);
    bool isConnected();
    void connect();
    void attemptReconnect();
    std::vector<Target> getTargets();
    std::vector<uint8_t> rawData();
    static void findLD2450();
    static String getClosestSensor();
    static void addScanResult(const std::string& address, int rssi);

private:
    static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);
    static void processBuffer();
    static Target transformTarget(const Target& originalTarget);
    static void scanCallback(BLEAdvertisedDevice advertisedDevice);

    static BLEClient* pClient;
    static BLERemoteCharacteristic* pRemoteCharacteristic;
    static std::vector<uint8_t> dataBuffer;
    static bool connected;
    static bool newDataAvailable;
    static BLEAddress sensorAddress;
    static int16_t offsetX, offsetY;
    static float rotationDegrees;
    static std::map<std::string, int> scanResults; // Stores scan results
};

#endif
