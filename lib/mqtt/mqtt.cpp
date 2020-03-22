#include "mqtt.h"


Mqtt::Mqtt(){
  configure();
}

void Mqtt::configure(){
    mqttClient.onConnect([&](bool sessionPresent)-> void {
      Serial.println("** Connected to the broker **");

    });

    mqttClient.onDisconnect([&](AsyncMqttClientDisconnectReason reason)-> void {
        Serial.println("** Disconnected from the broker **");
        reconnect();
    });

    mqttClient.onSubscribe([&](uint16_t packetId, uint8_t qos)-> void{
      Serial.println("** Subscribe acknowledged **");
    });

    mqttClient.onUnsubscribe([&](uint16_t packetId)-> void{
      Serial.println("** Unsubscribe acknowledged **");
    });

    mqttClient.onPublish([&](uint16_t packetId)-> void{
      Serial.println("** Publish acknowledged **");
    });

    mqttClient.onMessage([&](char* topic, char* payload,
                             AsyncMqttClientMessageProperties properties,
                             size_t len, size_t index, size_t total)-> void{
      Serial.println("** Publish received **");
    });
}

void Mqtt::reconnect(){
  Serial.println("** Reconnecting   to the broker **");
  mqttClient.connect();
}


void Mqtt::connect(char* mqttBroker, int mqttPort,char* mqttUser,char* mqttPass){
  mqttClient.setServer(mqttBroker, mqttPort);
  mqttClient.setKeepAlive(5).setCleanSession(false).setWill("topic/online", 2, true, "no").setCredentials(mqttUser, mqttPass).setClientId("Fuck");
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}