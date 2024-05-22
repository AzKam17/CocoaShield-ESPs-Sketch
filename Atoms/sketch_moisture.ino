#define soil_moisture_pin 4

void setup() {
  Serial.begin(9600);
}

void loop() {
  int sensorValue = analogRead(soil_moisture_pin);  // Read the analog value from sensor

  // Map sensor value to a percentage
  int percentage = map(sensorValue, 4095, 1580, 0, 100);

  Serial.print("Soil Moisture Percentage: ");
  Serial.println(percentage);

  delay(500);
}
