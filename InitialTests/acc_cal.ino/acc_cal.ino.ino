#include <Wire.h>
#include <MPU6050.h>
#include "BluetoothSerial.h"

MPU6050 mpu;
BluetoothSerial BT;

/* ================= USER SETTINGS ================= */
#define CAL_SAMPLES 1000
#define SERIAL_BAUD 115200
#define G_RAW 16384.0   // 1g at ±2g
/* ================================================= */

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Enable it in ESP32 menuconfig
#endif

/* Calibration coefficients */
float acc_m[3]; // scale
float acc_b[3]; // bias

/* Raw data holders */
int16_t ax, ay, az, gx, gy, gz;

/* ------------------------------------------------- */
void waitForUser(const char *msg) {
  Serial.println("--------------------------------------------------");
  Serial.println(msg);
  Serial.println("Press ENTER in Serial Monitor or send any char via Bluetooth");

  BT.println("--------------------------------------------------");
  BT.println(msg);
  BT.println("Send any character to continue");

  while (!Serial.available() && !BT.available()) {}
  while (Serial.available()) Serial.read();
  while (BT.available()) BT.read();

  delay(2000); // settle time
}

/* ------------------------------------------------- */
void collectAccelSamples(long &sx, long &sy, long &sz) {
  sx = sy = sz = 0;

  for (int i = 0; i < CAL_SAMPLES; i++) {
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    sx += ax;
    sy += ay;
    sz += az;
    delay(2);
  }
}

/* ------------------------------------------------- */
void calibrateAxis(int axis) {
  long sx, sy, sz;
  float pos, neg, zero;

  /* +1g */
  waitForUser(
    axis == 0 ? "Place +X axis UP" :
    axis == 1 ? "Place +Y axis UP" :
                "Place +Z axis UP"
  );
  collectAccelSamples(sx, sy, sz);
  pos = (axis == 0 ? sx : axis == 1 ? sy : sz) / (float)CAL_SAMPLES;

  /* -1g */
  waitForUser(
    axis == 0 ? "Place -X axis UP" :
    axis == 1 ? "Place -Y axis UP" :
                "Place -Z axis UP"
  );
  collectAccelSamples(sx, sy, sz);
  neg = (axis == 0 ? sx : axis == 1 ? sy : sz) / (float)CAL_SAMPLES;

  /* 0g */
  waitForUser(
    axis == 0 ? "Place X axis PERPENDICULAR to gravity" :
    axis == 1 ? "Place Y axis PERPENDICULAR to gravity" :
                "Place Z axis PERPENDICULAR to gravity"
  );
  collectAccelSamples(sx, sy, sz);
  zero = (axis == 0 ? sx : axis == 1 ? sy : sz) / (float)CAL_SAMPLES;

  /* Linear calibration */
  acc_m[axis] = (2.0 * G_RAW) / (pos - neg);
  acc_b[axis] = -acc_m[axis] * zero;

  Serial.printf("Axis %d -> scale: %.6f bias: %.2f\n",
                axis, acc_m[axis], acc_b[axis]);
  BT.printf("Axis %d -> scale: %.6f bias: %.2f\n",
            axis, acc_m[axis], acc_b[axis]);
}

/* ------------------------------------------------- */
void calibrateAccelerometer() {
  Serial.println("=== ACCELEROMETER CALIBRATION START ===");
  BT.println("=== ACCELEROMETER CALIBRATION START ===");

  for (int i = 0; i < 3; i++) {
    calibrateAxis(i);
  }

  Serial.println("=== CALIBRATION COMPLETE ===");
  BT.println("=== CALIBRATION COMPLETE ===");
}

/* ------------------------------------------------- */
void setup() {
  Serial.begin(SERIAL_BAUD);
  BT.begin("ESP32_MPU6050_CAL");

  Wire.begin();
  mpu.initialize();
  mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_8);


  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed!");
    BT.println("MPU6050 connection failed!");
    while (1);
  }

  #define PLOT_MODE 1

  Serial.println("MPU6050 connected");
  BT.println("MPU6050 connected");

  delay(2000);
  calibrateAccelerometer();
}

/* ------------------------------------------------- */
void loop() {
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  float ax_c = acc_m[0] * ax + acc_b[0];
  float ay_c = acc_m[1] * ay + acc_b[1];
  float az_c = acc_m[2] * az + acc_b[2];

  String raw = String(ax) + "," + String(ay) + "," + String(az);
  String cal = String(ax_c / G_RAW, 3) + "," +
               String(ay_c / G_RAW, 3) + "," +
               String(az_c / G_RAW, 3);

  #if PLOT_MODE
  Serial.print(ax_c / G_RAW);
  Serial.print(" ");
  Serial.print(ay_c / G_RAW);
  Serial.print(" ");
  Serial.println(az_c / G_RAW);
  #else
    Serial.println("RAW: " + raw);
    Serial.println("CAL g: " + cal);
  #endif

  BT.println("RAW: " + raw);
  BT.println("CAL g: " + cal);

  delay(1000);
}
