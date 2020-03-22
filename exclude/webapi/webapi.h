/* 
#ifndef Webapi_h
#define Webapi_h

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <FS.h>
#include <jsonInfo.h>

class Webapi
{
  public:
    Webapi();
    void begin();

  private:
    AsyncWebServer classWebServer = AsyncWebServer(80);
    // const char* http_username = "admin";
    // const char* http_password = "admin";
    void onRequestNotFound  (AsyncWebServerRequest *request);
    void onRequestTest      (AsyncWebServerRequest *request);
};

#endif */