#include <BleKeyboard.h>

const int hallPin = 23;  // GPIO pin connected to the hall sensor
BleKeyboard bleKeyboard("ESP32 Keyboard", "MyESP32", 100);  // Name and manufacturer

void setup() {
  Serial.begin(115200);
  pinMode(hallPin, INPUT);
  bleKeyboard.begin();
}

void loop() {
  int hallValue = digitalRead(hallPin);
  Serial.print("Hall Value: ");
  Serial.println(hallValue);  // Print hall sensor value

  if (bleKeyboard.isConnected()) {
    // Press "a" when the hall sensor is activated (adjust as needed)
    if (hallValue == HIGH) {
      Serial.println("Hall sensor activated, sending 'a'");
      bleKeyboard.press('a');
      delay(100);
      bleKeyboard.release('a');
    }
  }

  delay(1000);  // Check sensor state every 100 ms
}