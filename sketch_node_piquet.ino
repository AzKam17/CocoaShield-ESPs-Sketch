// Sketch pour le piquet

#include <Arduino.h>
#include "painlessMesh.h"
#include "DHT.h"

#define DHTPIN 17
#define DHTTYPE DHT11
#define soil_moisture_pin 4

#define MESH_PREFIX    "cocoashield_box_mesh" //name for your MESH
#define MESH_PASSWORD  "MESHpassword" //password for your MESH
#define MESH_PORT      5555 //default port

Scheduler userScheduler;
painlessMesh  mesh;
String readings;

void sendMessage() ;
String getReadings();
DHT dht(DHTPIN, DHTTYPE);


Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );

void sendMessage() {
  String msg = getReadings();
  Serial.println(msg);
  mesh.sendBroadcast( msg );
  taskSendMessage.setInterval((TASK_SECOND * 1, TASK_SECOND * 10));
}

String getReadings () {
  // Humidite de l'air
  float h = dht.readHumidity();
  // Temperature de l'air
  float t = dht.readTemperature();

  // Hmudite du sol
  int sensorValue = analogRead(soil_moisture_pin);
  int percentage = map(sensorValue, 4095, 1580, 0, 100);
  /*
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print("%  Temperature: ");
  Serial.print(t);
  Serial.print("°C, ");
  Serial.print("Soil Moisture Percentage: ");*/
  //Serial.println(sensorValue);

  DynamicJsonDocument doc(1024);
  doc["temp"] = t;
  doc["hum_air"] = h;
  //doc["hum_sol"] = percentage;
  doc["node"] = mesh.getNodeId();
  
  String msg ;
  serializeJson(doc, msg);
  return msg;
}

void setup() {
  Serial.begin(9600);
  dht.begin();
  
  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA, 6 );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  mesh.setContainsRoot(true);

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
}

void loop() {
  userScheduler.execute();
  mesh.update();
}

void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}
