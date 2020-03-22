/* 
#ifndef JsonInfo_h
#define JsonInfo_h

#include <Arduino.h>

#include <WiFi.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

class JsonInfo
{
  public:
    static void ScanWifiJson(JsonObject &json);
    static void InfoWifiAP(JsonObject &json);
    static void InfoWifiSTA(JsonObject &json);

  private:

    static void InfoSTA(JsonObject &json);
    static void InfoNetworkSTA(JsonObject &json);
    static String StringIpAddress(IPAddress address);
};

#endif */