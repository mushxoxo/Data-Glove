#include <Wire.h>
#include <MPU6050.h>
#include "BluetoothSerial.h"

MPU6050 mpu;
BluetoothSerial BT;  // Bluetooth Serial object

// Calibration parameters
const int CAL_SIZE = 500;   // same idea as Python
long gx_offset = 0;
long gy_offset = 0;
long gz_offset = 0;

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

    // Calibrate AFTER filter is set
    calibrateGyro();

    Serial.println("MPU6050 connected.");
    BT.println("MPU6050 connected.");
}


void loop() {
    int16_t ax, ay, az, gx, gy, gz;
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);\

    // Apply calibration
    gx -= gx_offset;
    gy -= gy_offset;
    gz -= gz_offset;
    
    String data = String(ax) + "," + String(ay) + "," + String(az) + "," + 
                  String(gx) + "," + String(gy) + "," + String(gz);
    
    Serial.println(data);
    BT.println(data); // Send data via Bluetooth

    delay(10);
}

void getGyro(int16_t &gx, int16_t &gy, int16_t &gz) {
    int16_t ax, ay, az;
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
}

void calibrateGyro() {
    Serial.println("--------------------------------------------------");
    Serial.println("Gyro Calibrating - Keep the IMU Steady");
    delay(2000);  // settle time (same intent as Python)

    // Clear buffer (same as Python pre-read)
    for (int i = 0; i < CAL_SIZE; i++) {
        int16_t gx, gy, gz;
        getGyro(gx, gy, gz);
        delay(2);
    }

    long gx_sum = 0;
    long gy_sum = 0;
    long gz_sum = 0;

    // Collect stationary samples
    for (int i = 0; i < CAL_SIZE; i++) {
        int16_t gx, gy, gz;
        getGyro(gx, gy, gz);

        gx_sum += gx;
        gy_sum += gy;
        gz_sum += gz;

        delay(2);  // ~500 Hz internal sampling
    }

    // Mean = bias
    gx_offset = gx_sum / CAL_SIZE;
    gy_offset = gy_sum / CAL_SIZE;
    gz_offset = gz_sum / CAL_SIZE;

    Serial.println("Gyro Calibration Complete");
    Serial.print("Offsets -> ");
    Serial.print("gx: "); Serial.print(gx_offset);
    Serial.print(" gy: "); Serial.print(gy_offset);
    Serial.print(" gz: "); Serial.println(gz_offset);
}


