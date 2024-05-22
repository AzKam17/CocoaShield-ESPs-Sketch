// Sketch pour le test de la connexion

#include <Arduino.h>
#include "painlessMesh.h"

#define LED 2

#define MESH_PREFIX    "cocoashield_box_mesh" //name for your MESH
#define MESH_PASSWORD  "MESHpassword" //password for your MESH
#define MESH_PORT      5555 //default port

Scheduler userScheduler;
painlessMesh  mesh;

void sendMessage();

Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );

void sendMessage() {
  digitalWrite(LED,HIGH);
  String msg = "Hello " + mesh.getNodeId();
  Serial.println(msg);
  mesh.sendBroadcast( msg );
  taskSendMessage.setInterval((TASK_SECOND * 1, TASK_SECOND * 10));
}

void setup() {
  Serial.begin(115200);
  
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
