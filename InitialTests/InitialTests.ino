const int hallPin1 = 18;
const int hallPin2 = 19;
const int hallPin4 = 21;
const int hallPin8 = 22;
const int hallPin16 = 23;

void setup() {
  Serial.begin(9600);  // Start serial communication at 9600 baud

  // Set each hall sensor pin as INPUT
  pinMode(hallPin1, INPUT);
  pinMode(hallPin2, INPUT);
  pinMode(hallPin4, INPUT_PULLUP);
  pinMode(hallPin8, INPUT);
  pinMode(hallPin16, INPUT);
}

void loop() {
  Serial.println("Hall Sensor Readings:");x

  // Read and print the value of each hall sensor
  int hallValue1 = digitalRead(hallPin1);
  Serial.print("Sensor on Pin ");
  Serial.print(hallPin1);
  Serial.print(": ");
  Serial.println(hallValue1);

  int hallValue2 = digitalRead(hallPin2);
  Serial.print("Sensor on Pin ");
  Serial.print(hallPin2);
  Serial.print(": ");
  Serial.println(hallValue2);

  int hallValue4 = digitalRead(hallPin4);
  Serial.print("Sensor on Pin ");
  Serial.print(hallPin4);
  Serial.print(": ");
  Serial.println(hallValue4);

  int hallValue8 = digitalRead(hallPin8);
  Serial.print("Sensor on Pin ");
  Serial.print(hallPin8);
  Serial.print(": ");
  Serial.println(hallValue8);

  int hallValue16 = digitalRead(hallPin16);
  Serial.print("Sensor on Pin ");
  Serial.print(hallPin16);
  Serial.print(": ");
  Serial.println(hallValue16);

  Serial.println();  // Print a blank line between readings
  delay(1000);  // Wait for 1 second before reading again
}
