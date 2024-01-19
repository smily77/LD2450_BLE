# LD2450_BLE
Arduino Library for the HLK-LD2450 Sensor using BLE (Read data)

Experimental state. THE ENTIRE LIBRARY WAS CODED BY GPT4!

This library provides the capability to read the HLK-LD2450 via BLE. The Arduino lib is made for an ESP32. Currently only tested on a ESP32 Dev Board

The methodes are implemented:

  void init(const char* address);
  
    initalize uses the MAC of the Sensor
    
  void connect();
  
  bool isConnected();
  
  void attemptReconnect();
  
    reconnection when connection was lost
    
  std::vector<Target> getTargets();
  
    provide the target informations from the sensor
    
  std::vector<uint8_t> rawData();
  
    provides the rawData from the sensor (30 Bytes)
    
  static void findLD2450();
  
    lists the Mac from all HLK-LD2450 sensors in view
    
  static String getClosestSensor();
  
    return the MAC of the HLK-LD2450 with the strogest RF signal
    
  static void addScanResult(const std::string& address, int rssi);
  
    used in findLD2450
    
  void setSensorOffset(int16_t offsetX, int16_t offsetY);
  
    sensor location in the coordinationsystem -> respected by target coordinates
    
  void setSensorRotation(float degrees);
  
    sensor rotation agains the coordinationsystem -> respected by target coordinates
