const int hallPin = 23;  // GPIO pin connected to the hall sensor
const int ledPin = 2;    // GPIO pin connected to the LED

void setup() {
  Serial.being(9600);
  pinMode(hallPin, INPUT);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  int hallValue = digitalRead(hallPin);
  if (hallValue == HIGH) {
    digitalWrite(ledPin, HIGH);  // Turn on LED if magnet is detected
  } else {
    digitalWrite(ledPin, LOW);   // Turn off LED otherwise
  }
  Serial.println(hallValue);
  delay(1000);
}