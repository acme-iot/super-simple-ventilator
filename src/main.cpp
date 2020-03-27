#include <fakes.h> // remove

extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}

#include <Arduino.h>
#include <Servo.h>

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

#include <ArduinoLog.h>

#ifdef RELEASE
#define LOG_LEVEL LOG_LEVEL_SILENT
#define HACK_MODE false
#else
#define LOG_LEVEL LOG_LEVEL_VERBOSE
#define HACK_MODE true
#endif

//some variables to tweek
#define version "20201603.1"
#define rate 16 // breathing cycles per minute
#define enable_motor true // useful for debugging without noise

#define max_speed 180
#define min_speed 0
#define PEEP_speed 40 //approx 5cm/H2O
#define led_pin 13
#define button_pin A5
#define current_pin A0
#define servo_pin 3
#define serial_baud 9600

int buttonState = 1;
int buttonStatePrev = 1;
int speed_state = 0;
int loop_count = 0;
int click_loop_count = 0;
int click_count = 0;

int target_speed_high = 0;
int target_speed_low = 0;

int cycle_counter = 0;
int cycle_phase = 0;
int mode = 0;
int current = 0;

const uint8_t ServoPin = A4;
Servo servo;

const int BaudRate = 115200;

const char *ssid = SECRET_SSID;
const char *password = SECRET_PASSWORD;

aquabotics::Configuration configuration{};
aquabotics::FileSystem fileSystem{};

// Timers
TimerHandle_t wifiReconnectTimer;
TimerHandle_t initializeBlowerTimer;

void connectToWifi() {
  Log.trace("connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
}

void initializeBlower() {
  Log.trace("initializing blower....");

}

void preInitialize() {
  assert(Serial);

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
        initializeBlowerTimer = xTimerCreate("blowerTimer",
                                             pdMS_TO_TICKS(1*1000),
                                             pdFALSE,
                                             (void *) 0,
                                             reinterpret_cast<TimerCallbackFunction_t >(initializeBlower));
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

void setupServo() {
  pinMode(ServoPin, OUTPUT);
}

void setup() {
  preInitialize();
  setupDebugging();
  setupLogging();
  setupFileSystem();
  setupWiFi();
  setupServo();

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

auto led_state = LOW;
void loop() {
  led_state = !led_state;

  digitalWrite(ServoPin, HIGH);
  delay(led_state==HIGH ? 6000 : 6000);
  digitalWrite(ServoPin, LOW);
  delay(3000);

  digitalWrite(LED_BUILTIN, led_state);
}