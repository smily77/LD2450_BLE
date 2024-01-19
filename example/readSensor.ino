// 76:e7:80:75:1d:29
// nun mit Sensor erkennen
#include "LD2450_BLE.h"


LD2450_BLE bleDevice;

void setup() {
    Serial.begin(115200);
    LD2450_BLE::findLD2450(); // Startet den Scan nach HLK-LD2450-Geräten
    delay(12000); // Warte bis der Scan abgeschlossen ist

    String closestSensorMAC = LD2450_BLE::getClosestSensor();
    if (!closestSensorMAC.isEmpty()) {
        Serial.print("Closest Sensor MAC: ");
        Serial.println(closestSensorMAC);
        bleDevice.init(closestSensorMAC.c_str()); // Initialisiere das Gerät mit der nächsten Sensor-MAC
    }

    bleDevice.setSensorOffset(10, 20);
    bleDevice.setSensorRotation(45);
    bleDevice.connect();
}

void loop() {
    if (bleDevice.isConnected()) {
        auto targets = bleDevice.getTargets();
        for (const auto& target : targets) {
            if (target.active) {
                Serial.print("Transformed Target: X=");
                Serial.print(target.x);
                Serial.print(", Y=");
                Serial.print(target.y);
                Serial.print(", Speed=");
                Serial.print(target.speed);
                Serial.print(", Radius=");
                Serial.print(target.radius);
                Serial.print(", Angle=");
                Serial.println(target.angle);
            }
        }
    } else {
        Serial.println("Not connected. Attempting to reconnect...");
        bleDevice.attemptReconnect();
        delay(5000); // Wait 5 seconds before trying to reconnect
    }
    delay(1000);
}
