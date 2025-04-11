#include <Wire.h>
#include <MPU6050.h>
#include "BluetoothSerial.h"

MPU6050 mpu;
BluetoothSerial BT;  // Bluetooth Serial object

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif


void setup() {
    Serial.begin(115200);
    BT.begin("ESP32_MPU6050"); // Bluetooth device name
    Wire.begin();
    mpu.initialize();

    if (!mpu.testConnection()) {
        Serial.println("MPU6050 connection failed!");
        BT.println("MPU6050 connection failed!");
        while (1);
    }

    Serial.println("MPU6050 connected.");
    BT.println("MPU6050 connected.");
}


void loop() {
    int16_t ax, ay, az, gx, gy, gz;
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    
    String data = String(ax) + "," + String(ay) + "," + String(az) + "," + 
                  String(gx) + "," + String(gy) + "," + String(gz);
    
    Serial.println(data);
    BT.println(data); // Send data via Bluetooth

    delay(10);
}

