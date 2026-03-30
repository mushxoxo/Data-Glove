#include <Wire.h>
#include "BluetoothSerial.h"

BluetoothSerial BT;  // Bluetooth Serial object

const int MPU_addr = 0x68;

// ===== Raw Data =====
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;

// ===== Calibration =====
const int CAL_SIZE = 500;

float gx_offset = 0;
float gy_offset = 0;
float gz_offset = 0;

// Accelerometer calibration (from Maker Portal method)


// Accelerometer calibration (AFS_SEL = ±2g)
// const float acc_m[3] = {0.998609, 0.998455, 0.976508};
// const float acc_b[3] = {-412.61, -347.75, -161.62};

// Accelerometer calibration (AFS_SEL = ±8g)
// const float acc_m[3] = {
//     3.994889,   // X scale
//     3.990031,   // Y scale
//     3.911422    // Z scale
// };

const float acc_b[3] = {
     41.24,
   1290.67,
  -1027.07
};

const float G_RAW = 16384.0;

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

// ===== Timing =====
const int SAMPLE_DELAY = 10; // 10ms → 100Hz

void setup() {
    Serial.begin(115200);
    Wire.begin();
    BT.begin("Cyber-Glove");

    // Wake up MPU
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x6B);  
    Wire.write(0x00);  
    Wire.endTransmission(true);

    // // Accelerometer
    // mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_8);
    // Equivalent raw register:
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x1C);  // ACCEL_CONFIG
    Wire.write(0x10);  // ±8g
    Wire.endTransmission(true);

    // // Gyroscope
    // mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_1000);
    // Equivalent raw register:
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x1B);  // GYRO_CONFIG
    Wire.write(0x10);  // ±1000 dps
    Wire.endTransmission(true);

    Serial.println("MPU initialized (raw mode)");
    BT.println("MPU initialized (raw mode)");

    delay(2000);

    calibrateGyro();
}

void loop() {
    unsigned long start = millis();

    // Read 14 registers
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_addr, 14);

    int16_t t = Wire.read();
    AcX = (t << 8) | Wire.read();

    t = Wire.read();
    AcY = (t << 8) | Wire.read();

    t = Wire.read();
    AcZ = (t << 8) | Wire.read();

    t = Wire.read();
    Tmp = (t << 8) | Wire.read();

    t = Wire.read();
    GyX = (t << 8) | Wire.read();

    t = Wire.read();
    GyY = (t << 8) | Wire.read();

    t = Wire.read();
    GyZ = (t << 8) | Wire.read();

    // ===== Convert to usable units =====
    float ax = AcX / 4096.0;   // ±8g → 4096 LSB/g
    float ay = AcY / 4096.0;
    float az = AcZ / 4096.0;

    float gx = GyX / 32.8;     // ±1000 dps → 32.8 LSB/dps
    float gy = GyY / 32.8;
    float gz = GyZ / 32.8;

    // ===== Apply gyro offset =====
    gx -= gx_offset;
    gy -= gy_offset;
    gz -= gz_offset;

    // ===== Output format =====
    String data =
        String(ax, 4) + "," +
        String(ay, 4) + "," +
        String(az, 4) + "," +
        String(gx, 4) + "," +
        String(gy, 4) + "," +
        String(gz, 4);

    Serial.println(data);
    BT.println(data);

    // ===== Maintain 100Hz =====
    while (millis() - start < SAMPLE_DELAY);
}

void calibrateGyro() {
    Serial.println("--------------------------------------------------");
    Serial.println("Calibrating Gyro... Keep sensor STILL");

    delay(2000);

    float gx_sum = 0;
    float gy_sum = 0;
    float gz_sum = 0;

    for (int i = 0; i < CAL_SIZE; i++) {

        Wire.beginTransmission(MPU_addr);
        Wire.write(0x43);
        Wire.endTransmission(false);
        Wire.requestFrom(MPU_addr, 6);

        int16_t gx = (Wire.read() << 8) | Wire.read();
        int16_t gy = (Wire.read() << 8) | Wire.read();
        int16_t gz = (Wire.read() << 8) | Wire.read();

        gx_sum += gx / 32.8;
        gy_sum += gy / 32.8;
        gz_sum += gz / 32.8;

        delay(2);
    }

    gx_offset = gx_sum / CAL_SIZE;
    gy_offset = gy_sum / CAL_SIZE;
    gz_offset = gz_sum / CAL_SIZE;

    Serial.println("Gyro Calibration Done");
    Serial.print("Offsets: ");
    Serial.print(gx_offset); Serial.print(", ");
    Serial.print(gy_offset); Serial.print(", ");
    Serial.println(gz_offset);
}