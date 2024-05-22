// Sketch du piquet avec SD Card

#include <SPI.h>
#include <SD.h>
#include "DHT.h"

#define DHTPIN 17
#define DHTTYPE DHT11
#define soil_moisture_pin 4
#define CS 5

DHT dht(DHTPIN, DHTTYPE);
File myFile;

void WriteFile(const char * path, const char * message){
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open(path, FILE_APPEND);
  // if the file opened okay, write to it:
  if (myFile) {
    Serial.printf("Writing to %s ", path);
    myFile.println(message);
    myFile.close(); // close the file:
    Serial.println("completed.");
  }
  // if the file didn't open, print an error:
  else {
    Serial.println("error opening file ");
    Serial.println(path);
  }
}

void setup() {
  Serial.begin(9600);
  dht.begin();
  delay(500);
  while (!Serial) { ; }
  Serial.println("Initializing SD card...");
  if (!SD.begin(CS)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
}

void loop() {
  delay(1000 * 60 * 2);
  // Read temperature and humidity from DHT11 sensor
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    Serial.println("Echec reception");
    return;
  }
  // Read soil moisture from analog sensor and map to percentage
  int sensorValue = analogRead(soil_moisture_pin);
  int percentage = map(sensorValue, 4095, 1580, 0, 100);
  // Print the readings to the serial monitor
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print("%  Temperature: ");
  Serial.print(t);
  Serial.print("°C, ");
  Serial.print("Soil Moisture Percentage: ");
  Serial.println(percentage);
  // Write the readings to the SD card
  char message[100];
  snprintf(message, 100, "%.1f, %.1f, %d", h, t, percentage);
  WriteFile("/data.txt", message);
}
