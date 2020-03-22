#include <fakes.h> // remove

#include <Arduino.h>

#include <WiFi.h>
#include <WiFiType.h>
#include <TimeLib.h>

#include <chrono>
#include <ctime>
#include <string>
#include <cassert>

#include <FileSystem.h>
#include <Configuration.h>

#ifndef CI_BUILD
#include "secrets.h"
#else
#define SECRET_SSID "SSID"
#define SECRET_PASSWORD "password1234"
#define SECRET_MQTT_USERNAME ""
#define SECRET_MQTT_PASSWORD ""
#endif

extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}

#include <ArduinoLog.h>

#ifdef RELEASE
#define LOG_LEVEL LOG_LEVEL_SILENT
#define HACK_MODE false
#else
#define LOG_LEVEL LOG_LEVEL_VERBOSE
#define HACK_MODE true
#endif

#undef LOW
#define LOW 0x1
#undef HIGH
#define HIGH 0x0


const int BaudRate = 115200;

const char *ssid = SECRET_SSID;
const char *password = SECRET_PASSWORD;

aquabotics::Configuration configuration{};
aquabotics::FileSystem fileSystem{};

// Timers
TimerHandle_t wifiReconnectTimer;
TimerHandle_t timeTimer;
TimerHandle_t configurationTimer;

void connectToWifi() {
  Log.trace("connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
}

void preInitialize() {
  // assert all that is required
  assert(Serial);

  // init Serial
  Serial.begin(BaudRate);
  while (!Serial && !Serial.available()) {
  }
}

void setupDebugging() {
  Serial.setDebugOutput(true);
}

void setupLogging() {
  auto getFormattedTime = [](Print *p) {
    auto now = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::high_resolution_clock::to_time_t(now);
    auto localTime = std::localtime(&time);
    auto str = std::string(asctime(localTime));
    str.pop_back();
    str += " ";
    p->print(str.c_str());
  };

  Log.begin(LOG_LEVEL, &Serial);
  Log.setPrefix(getFormattedTime);
  Log.setSuffix([](Print *p) { p->print("\n"); });
}

void setupFileSystem() {
  Log.trace("setting up file system");
  fileSystem.begin();

  Log.trace("setting up configuration");
  configuration.begin();

  //Log.trace("starting refresh configuration timer");
  /*configurationTimer = xTimerCreate("configurationTimer",
                                    pdMS_TO_TICKS(10*1000),
                                    pdFALSE,
                                    (void *) 0,
                                    reinterpret_cast<TimerCallbackFunction_t>(requestGatewayConfiguration));*/

  /*if (xTimerStart(configurationTimer, 0)!=pdPASS) {
    Log.fatal("Configuration timer failed.");
  }*/
}

void setupWiFi() {

  auto handleEvents = [](WiFiEvent_t event) {
    Log.trace("[WiFi-event] event: %d", event);
    switch (event) {
      case SYSTEM_EVENT_STA_GOT_IP: {
        Log.trace("wifi connected, %s", WiFi.localIP().toString().c_str());
        // connect/expose services
        break;
      }
      case SYSTEM_EVENT_STA_DISCONNECTED: {
        Log.error("WiFi lost connection");
        xTimerStart(wifiReconnectTimer, 0);
        break;
      }
      case SYSTEM_EVENT_WIFI_READY: {
        Log.trace("SYSTEM_EVENT_WIFI_READY");
        break;
      }
      case SYSTEM_EVENT_SCAN_DONE: {
        Log.trace("SYSTEM_EVENT_SCAN_DONE");
        break;
      }
      case SYSTEM_EVENT_STA_START: {
        Log.trace("SYSTEM_EVENT_STA_START");
        break;
      }
      case SYSTEM_EVENT_STA_STOP: {
        Log.trace("SYSTEM_EVENT_STA_STOP");
        break;
      }
      case SYSTEM_EVENT_STA_CONNECTED: {
        Log.trace("SYSTEM_EVENT_STA_CONNECTED");
        break;
      }
      case SYSTEM_EVENT_STA_AUTHMODE_CHANGE: {
        Log.trace("SYSTEM_EVENT_STA_AUTHMODE_CHANGE");
        break;
      }
      case SYSTEM_EVENT_STA_LOST_IP: {
        Log.trace("SYSTEM_EVENT_STA_LOST_IP");
        break;
      }
      case SYSTEM_EVENT_STA_WPS_ER_SUCCESS: {
        Log.trace("SYSTEM_EVENT_STA_WPS_ER_SUCCESS");
        break;
      }
      case SYSTEM_EVENT_STA_WPS_ER_FAILED: {
        Log.trace("SYSTEM_EVENT_STA_WPS_ER_FAILED");
        break;
      }
      case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT: {
        Log.trace("SYSTEM_EVENT_STA_WPS_ER_TIMEOUT");
        break;
      }
      case SYSTEM_EVENT_STA_WPS_ER_PIN: {
        Log.trace("SYSTEM_EVENT_STA_WPS_ER_PIN");
        break;
      }
      case SYSTEM_EVENT_STA_WPS_ER_PBC_OVERLAP: {
        Log.trace("SYSTEM_EVENT_STA_WPS_ER_PBC_OVERLAP");
        break;
      }
      case SYSTEM_EVENT_AP_START: {
        Log.trace("SYSTEM_EVENT_AP_START");
        break;
      }
      case SYSTEM_EVENT_AP_STOP: {
        Log.trace("SYSTEM_EVENT_WIFI_READY");
        break;
      }
      case SYSTEM_EVENT_AP_STACONNECTED: {
        Log.trace("SYSTEM_EVENT_AP_STACONNECTED");
        break;
      }
      case SYSTEM_EVENT_AP_STADISCONNECTED: {
        Log.trace("SYSTEM_EVENT_AP_STADISCONNECTED");
        break;
      }
      case SYSTEM_EVENT_AP_STAIPASSIGNED: {
        Log.trace("SYSTEM_EVENT_AP_STAIPASSIGNED");
        break;
      }
      case SYSTEM_EVENT_AP_PROBEREQRECVED: {
        Log.trace("SYSTEM_EVENT_AP_PROBEREQRECVED");
        break;
      }
      case SYSTEM_EVENT_GOT_IP6: {
        Log.trace("SYSTEM_EVENT_GOT_IP6");
        break;
      }
      case SYSTEM_EVENT_ETH_START:
      case SYSTEM_EVENT_ETH_STOP:
      case SYSTEM_EVENT_ETH_CONNECTED:
      case SYSTEM_EVENT_ETH_DISCONNECTED:
      case SYSTEM_EVENT_ETH_GOT_IP: {
        Log.trace("SYSTEM_EVENT_ETH_*");
        break;
      }
      case SYSTEM_EVENT_MAX: {
        Log.trace("SYSTEM_EVENT_MAX");
        break;
      }
    }
  };

  auto scanNetworks = [](String expectedSSID) {
    Log.trace("scanNetworks::begin");
    auto ssidCount = -1;
    while (ssidCount = WiFi.scanNetworks(), ssidCount==-1) {
      Log.error("scanNetworks::couldn't get a wifi connection");
    }

    Log.trace("scanNetworks::network count, %i", ssidCount);

    for (auto i = 0; i < ssidCount; i++) {
      Log.trace("scanNetworks::found %s (%idBm) %s", WiFi.SSID(i).c_str(), WiFi.RSSI(i), ""/*WiFi.encryptionType(i)*/);
      if (WiFi.SSID(i)==expectedSSID) {
        return true;
      }
    }
    return false;
  };

  WiFi.onEvent(handleEvents);
  while (!scanNetworks(ssid) && !HACK_MODE) {
    Log.error("unable to locate SSID %s", ssid);
    delay(1000);
  }

  connectToWifi();

}

void setup() {
  preInitialize();
  setupDebugging();
  setupLogging();
  setupFileSystem();
  setupWiFi();

  Log.notice("Running...");

  /*timeTimer = xTimerCreate("timeTimer",
                           pdMS_TO_TICKS(30*1000),
                           pdTRUE,
                           (void *) 0,
                           reinterpret_cast<TimerCallbackFunction_t>(syncTimeCallback));


  if (xTimerStart(timeTimer, 0)!=pdPASS) {
    Log.error("Time timer failed.");
  }*/

  pinMode(LED_BUILTIN, OUTPUT);

  Log.trace("MAC: %x", WiFi.macAddress().c_str());

  wifiReconnectTimer = xTimerCreate("wifiTimer",
                                    pdMS_TO_TICKS(1*1000),
                                    pdFALSE,
                                    (void *) 0,
                                    reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

  connectToWifi();
}

void loop() {
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
}