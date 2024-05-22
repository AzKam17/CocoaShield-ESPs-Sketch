// Sketch pour le pont de connexion entre les ESP et le Pi

#include <Arduino.h>
#include "painlessMesh.h"
#include <Arduino_JSON.h>
#include <PubSubClient.h>
#include <WiFiClient.h>

#define LED 2

#define MESH_PREFIX    "cocoashield_box_mesh" //name for your MESH
#define MESH_PASSWORD  "MESHpassword" //password for your MESH
#define MESH_PORT      5555 //default port

#define   STATION_SSID     "cocoashield_box_1"
#define   STATION_PASSWORD "ChangeMe"

#define HOSTNAME "cs1.local"

// Prototypes
void receivedCallback( const uint32_t &from, const String &msg );
void newConnectionCallback( const uint32_t &nodeId);
void mqttCallback(char* topic, byte* payload, unsigned int length);


IPAddress getlocalIP();

IPAddress myIP(0,0,0,0);
IPAddress mqttBroker(10, 3, 141 ,1);

painlessMesh  mesh;
WiFiClient wifiClient;
PubSubClient mqttClient(mqttBroker, 1883, mqttCallback, wifiClient);

void setup() {
  Serial.begin(115200);

  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );  // set before init() so that you can see startup messages

  // Channel set to 6. Make sure to use the same channel for your mesh and for you other
  // network (STATION_SSID)
  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, 6 );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);

  mesh.stationManual(STATION_SSID, STATION_PASSWORD);
  mesh.setHostname(HOSTNAME);

  // Bridge node, should (in most cases) be a root node. See [the wiki](https://gitlab.com/painlessMesh/painlessMesh/wikis/Possible-challenges-in-mesh-formation) for some background
  mesh.setRoot(true);
  // This node and all other nodes should ideally know the mesh contains a root, so call this on all nodes
  mesh.setContainsRoot(true);

  pinMode(LED,OUTPUT);
}

void loop() {
  mesh.update();
  mqttClient.loop();

  if(myIP != getlocalIP()){
    myIP = getlocalIP();
    Serial.println("My IP is " + myIP.toString());

    if (mqttClient.connect("painlessMeshClient", "azk", "Mosquitto")) {
      mqttClient.publish("painlessMesh/from/gateway","Ready!");
      mqttClient.subscribe("painlessMesh/to/#");
    } 
  }
}

void receivedCallback( const uint32_t &from, const String &msg ) {
  Serial.printf("bridge: Received from %u msg=%s\n", from, msg.c_str());
  String topic = "sensors/all";
  mqttClient.publish(topic.c_str(), msg.c_str());
  digitalWrite(LED,HIGH);
  delay(200);
  digitalWrite(LED,LOW);
  delay(100);
  digitalWrite(LED,HIGH);
  delay(200);
  digitalWrite(LED,LOW);
}


void newConnectionCallback( const uint32_t &nodeId){
  Serial.printf("bridge: Connection from %u ---", nodeId);
}

void mqttCallback(char* topic, uint8_t* payload, unsigned int length) {
  char* cleanPayload = (char*)malloc(length+1);
  memcpy(cleanPayload, payload, length);
  cleanPayload[length] = '\0';
  String msg = String(cleanPayload);
  free(cleanPayload);

  String targetStr = String(topic).substring(16);

  if(targetStr == "gateway")
  {
    if(msg == "getNodes")
    {
      auto nodes = mesh.getNodeList(true);
      String str;
      for (auto &&id : nodes)
        str += String(id) + String(" ");
      mqttClient.publish("painlessMesh/from/gateway", str.c_str());
    }
  }
  else if(targetStr == "broadcast") 
  {
    mesh.sendBroadcast(msg);
  }
  else
  {
    uint32_t target = strtoul(targetStr.c_str(), NULL, 10);
    if(mesh.isConnected(target))
    {
      mesh.sendSingle(target, msg);
    }
    else
    {
      mqttClient.publish("painlessMesh/from/gateway", "Client not connected!");
    }
  }
}


IPAddress getlocalIP() {
  return IPAddress(mesh.getStationIP());
}

