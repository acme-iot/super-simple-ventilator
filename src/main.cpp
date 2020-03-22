/*
 * Aquaponic planter firmware
 */
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
#include <AsyncMqttClient.h>

#ifdef RELEASE
#define LOG_LEVEL LOG_LEVEL_SILENT
#define HACK_MODE false
#else
#define LOG_LEVEL LOG_LEVEL_VERBOSE
#define HACK_MODE true
#endif

const int BaudRate = 115200;

const char *ssid = SECRET_SSID;
const char *password = SECRET_PASSWORD;
const char *mqtt_username = SECRET_MQTT_USERNAME;
const char *mqtt_password = SECRET_MQTT_PASSWORD;

#define MQTT_PORT 1883
#define MQTT_HOST IPAddress(54, 70, 96, 251)

aquabotics::Configuration configuration{};
aquabotics::FileSystem fileSystem{};
AsyncMqttClient asyncMqttClient;

// Timers
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;
TimerHandle_t timeTimer;
TimerHandle_t configurationTimer;

void connectToWifi() {
  Log.trace("connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
}

void connectToMqtt() {
  Log.trace("Connecting to MQTT...");

  auto mqttHost = IPAddress();
  mqttHost.fromString(configuration.getMqttIp());
  asyncMqttClient.setServer(/*MQTT_HOST*/mqttHost, /*MQTT_PORT*/configuration.getMqttPort());
  asyncMqttClient.setCredentials(configuration.getMqttUsername().c_str(), configuration.getMqttPassword().c_str());

  asyncMqttClient.connect();
}

void requestGatewayConfiguration() {
  Log.trace("Fetching configuration from gateway...");

  // http request to gateway
  String response = CONFIG_JSON_RESPONSE;

  try {
    configuration.load(response);
  } catch (std::exception &e) {
    Log.error("failed to load json, %s", e.what());
    throw e;
  }

  if (configuration.hasChanged(true)) {
    // refresh settings
    // refresh mqtt
  }
}

void syncTimeCallback() {
  Log.trace("Connecting to NTP server...");
  Log.trace("%s:%i", configuration.getMqttIp().c_str(), configuration.getMqttPort());
  return;
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  //Serial.print("Session present: ");
  //Serial.println(sessionPresent);
  uint16_t packetIdSub = asyncMqttClient.subscribe("test/lol", 2);
  //Serial.print("Subscribing at QoS 2, packetId: ");
  //Serial.println(packetIdSub);
  asyncMqttClient.publish("test/lol", 0, true, "test 1");
  //Serial.println("Publishing at QoS 0");
  uint16_t packetIdPub1 = asyncMqttClient.publish("test/lol", 1, true, "test 2");
  //Serial.print("Publishing at QoS 1, packetId: ");
  //Serial.println(packetIdPub1);
  uint16_t packetIdPub2 = asyncMqttClient.publish("test/lol", 2, true, "test 3");
  Serial.print("Publishing at QoS 2, packetId: ");
  Serial.println(packetIdPub2);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    xTimerStart(mqttReconnectTimer, 0);
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void onMqttMessage(char *topic,
                   char *payload,
                   AsyncMqttClientMessageProperties properties,
                   size_t len,
                   size_t index,
                   size_t total) {
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);
  Serial.print("  payload: ");
  Serial.println(payload);
}

void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
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

/*void loadConfiguration() {
  Log.notice("loading saved configuration");
  configuration.load();

  if (configuration.hasInitialized()) {
    return;
  }

  //fetch from gateway
  //requestGatewayConfiguration();
}*/

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
    Serial.printf("[WiFi-event] event: %d\n", event);
    switch (event) {
      case SYSTEM_EVENT_STA_GOT_IP: {
        Log.trace("wifi connected, %s", WiFi.localIP().toString().c_str());
        requestGatewayConfiguration();
        connectToMqtt();
        break;
      }
      case SYSTEM_EVENT_STA_DISCONNECTED: {
        Log.error("WiFi lost connection");
        xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
        xTimerStop(configurationTimer, 0);
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

  //fileSystem.deleteFile("/config.json");

  //loadConfiguration();

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

  //g_cfg->loadConfig();

  // FILE
  /*File file = SPIFFS.open("/config.json", FILE_WRITE);

  if (!file) {
    Log.error("There was an error opening the file for writing");
    return;
  }

  if (file.print("TEST")) {
    Log.trace("File was written");
  } else {
    Log.error("File write failed");
  }

  file.close();

  file = SPIFFS.open("/config.json", FILE_READ);
  Log.verbose("config.json %s", file.readString().c_str());
  file.close();

  SPIFFS.remove("/config.json");

  if (SPIFFS.exists("/config.json")) {

    Log.trace("Exists");
  }*/
  // END FILE

  // FILESYSTEM
  // END FILESYSTEM

  // Scan WiFi

  //WiFi.scanNetworks()

  // end Wifi


  mqttReconnectTimer = xTimerCreate("mqttTimer",
                                    pdMS_TO_TICKS(30*1000),
                                    pdFALSE,
                                    (void *) 0,
                                    reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  wifiReconnectTimer = xTimerCreate("wifiTimer",
                                    pdMS_TO_TICKS(1*1000),
                                    pdFALSE,
                                    (void *) 0,
                                    reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

  asyncMqttClient.onConnect(onMqttConnect);
  asyncMqttClient.onDisconnect(onMqttDisconnect);
  asyncMqttClient.onSubscribe(onMqttSubscribe);
  asyncMqttClient.onUnsubscribe(onMqttUnsubscribe);
  asyncMqttClient.onMessage(onMqttMessage);
  asyncMqttClient.onPublish(onMqttPublish);

  connectToWifi();
}

void loop() {
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
}