/*
*/
#ifndef Mqtto_h
#define Mqtto_h

#include <Arduino.h>
#include <AsyncMqttClient.h>
#include <ArduinoJson.h>

class Mqtt
{
  public:
  Mqtt();
  void connect(char* mqttBroker, int mqttPort,char* mqttUser,char* mqttPass);
  void reconnect();
  private:
    AsyncMqttClient mqttClient;
    void configure();
    void onMqttConnect(bool sessionPresent);
};

#endif