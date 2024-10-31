#include <BleKeyboard.h>
#include <Wire.h>
#include <MPU6050.h>

const int hallPin = 23;  // GPIO pin connected to the hall sensor
BleKeyboard bleKeyboard("ESP32 Keyboard", "MyESP32", 100);  // Name and manufacturer
MPU6050 mpu;
int16_t ax, ay, az;

void setup() {
  Serial.begin(115200);
  pinMode(hallPin, INPUT);
  bleKeyboard.begin();

  // Initialize the MPU6050
  Wire.begin();
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed");
    while (1); // Stop here if MPU6050 initialization fails
  }
}

void loop() {
  int hallValue = digitalRead(hallPin);
  Serial.print("Hall Value: ");
  Serial.println(hallValue);  // Print hall sensor value

  // Read MPU6050 orientation
  mpu.getAcceleration(&ax, &ay, &az);
  
  // Calculate angles in degrees
  float angleX = atan2(ay, az) * 180 / PI;
  float angleY = atan2(ax, az) * 180 / PI;
  float angleZ = atan2(ay, ax) * 180 / PI;

  // Print X, Y, Z angles
  Serial.print("Angle X: ");
  Serial.print(angleX);
  Serial.print(" | Angle Y: ");
  Serial.print(angleY);
  Serial.print(" | Angle Z: ");
  Serial.println(angleZ);

  if (bleKeyboard.isConnected() && hallValue == HIGH) {
    // Send 'a' if device is flat and hall sensor is high
    if (angleY > -5 && angleY < 5) {
      Serial.println("Device flat, sending 'a'");
      bleKeyboard.press('a');
      delay(100);
      bleKeyboard.release('a');
    }
    // Send 'b' if device is tilted 90 degrees on Y-axis and hall sensor is high
    else if (angleY > 85 && angleY < 95) {
      Serial.println("Device tilted 90 degrees, sending 'b'");
      bleKeyboard.press('b');
      delay(100);
      bleKeyboard.release('b');
    }
  }

  delay(1000);  // Check sensor state every second
}
