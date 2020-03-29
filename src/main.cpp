#include <Arduino.h>

#include <ArduinoLog.h>
#include <TimeLib.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include "SSD1306Wire.h"

#include <chrono>
#include <ctime>
#include <string>
//#include <cassert>

#ifndef CI_BUILD
#include "secrets.h"
#else
#define SECRET_SSID "SSID"
#define SECRET_PASSWORD "password1234"
#define SECRET_MQTT_USERNAME ""
#define SECRET_MQTT_PASSWORD ""
#endif

#ifdef RELEASE
#define LOG_LEVEL LOG_LEVEL_SILENT
#define HACK_MODE false
#else
#define LOG_LEVEL LOG_LEVEL_VERBOSE
#define HACK_MODE true
#endif

//some variables to tweek
#define version "20201603.1"
//#define normailizedRate 16 // breathing cycles per minute
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
int click_loop_count = 0;
int click_count = 0;

int target_speed_high = 0;
int target_speed_low = 0;

int mode = 0;
int current = 0;

const String Version = "0.0.1";
const String AccessPointSSID = "iot-ap";

const uint8_t PotPin = A0;
const uint8_t ControllerPin = D5;
const uint8_t FlashPin = 0;

const uint8_t CycleLow = 13;
const uint8_t CycleHigh = 20;

const int BaudRate = 115200;

const char *ssid = SECRET_SSID;
const char *password = SECRET_PASSWORD;

const int LineHeight = 16;

SSD1306Wire display(0x3c, SDA, SCL);

WiFiManager wifiManager;

int loop_count = 0;
int cycle_counter = 0;
int cycle_phase = 0;

int line = LineHeight;
bool overwrite = false;

/*void connectToWifi() {
  Log.trace("connecting to Wi-Fi...");
}

void initializeBlower() {
  Log.trace("initializing blower....");

}
*/

//helper method for oLed
//pout -> Print Out
void pout(String msg) {
  if (overwrite==true) {
    display.setColor(BLACK);
    display.fillRect(0, line, 128, 64);
    display.setColor(WHITE);
    display.drawString(0, line, msg);
    display.display();
    line += LineHeight;
  } else {
    display.drawString(0, line, msg);
    display.display();
    line += LineHeight;
  }

  if (line>=LineHeight*3) {
    overwrite = true;
    line = LineHeight * 2;
  }
}

void wifiManagerApCallback(WiFiManager *manager) {
  const String ssid = manager->getConfigPortalSSID();
  Log.trace("config mode: IP %s\nSSIF %s",
            WiFi.softAPIP().toString().c_str(),
            ssid.c_str()
  );
  pout("Connected to WiFi");
  pout("  " + ssid);
  //digitalWrite(LED_BUILTIN, LOW);
  delay(2000);
}

void preInitialize() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(FlashPin, INPUT);
  pinMode(PotPin, INPUT);
  pinMode(ControllerPin, OUTPUT);

  // Quickly kill motor
  digitalWrite(ControllerPin, LOW);

  Serial.begin(BaudRate);
  while (!Serial && !Serial.available()) {
  }
}

void printBoardInfo() {
  Log.verbose("ESP INFO");
  Log.verbose("\tVoltage:\t%d", ESP.getVcc());
  Log.verbose("\tBootMode:\t%d", ESP.getBootMode());
  Log.verbose("\tBootVersion:\t%d", ESP.getBootVersion());
  Log.verbose("\tChipId:\t%d", ESP.getChipId());
  Log.verbose("\tCoreVersion:\t%s", ESP.getCoreVersion().c_str());
  Log.verbose("\tCpuFrequencyMHz:\t%d", ESP.getCpuFreqMHz());
  Log.verbose("\tFullVersion:\t%s", ESP.getFullVersion().c_str());
  Log.verbose("\tResetReason:\t%s", ESP.getResetReason().c_str());
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

void setupWiFi() {
  wifiManager.setDebugOutput(HACK_MODE);
  wifiManager.setAPCallback(wifiManagerApCallback);
}

void setupDisplay() {
  display.init();
  display.flipScreenVertically();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "Ventilator " + Version);
  display.display();
  line += LineHeight;
  delay(3000);
}
/*
void setupServo() {
  pinMode(ServoPin, OUTPUT);
}*/

void setup() {
  preInitialize();
  setupDebugging();
  setupLogging();
  printBoardInfo();
  setupWiFi();
  setupDisplay();
  /*setupServo();
 */

  Log.notice("Running...");

  digitalWrite(LED_BUILTIN, HIGH);

  /*connectToWifi();*/
}

void initializeWiFi() {
  if (!wifiManager.startConfigPortal(AccessPointSSID.c_str())) {
    Log.error("failed to connect to ", AccessPointSSID.c_str());
    delay(3000);
    ESP.reset();
    delay(5000);
  }
}


void loop() {

  if (digitalRead(FlashPin)==LOW) {
    Log.trace("initializing WiFi");
    initializeWiFi();
  }

  const int potValue = analogRead(A0);
  const int normailizedRate = map(potValue, 0, 1024, CycleLow, CycleHigh);
  pout("Cycles " + String(normailizedRate) + "/min");

  // handle breath in/out cycle at target rate/min
  cycle_counter += 1;
  if( (30*100)/normailizedRate < cycle_counter){
    cycle_phase = (cycle_phase+1)%2;
    cycle_counter = 0;
    //Log.trace(loop_count/100.0);
    //Log.trace("\tphase speed:");
    if(cycle_phase == 0){
      if(enable_motor) {
        Log.trace("CONTROLLER ON");
        digitalWrite(ControllerPin, HIGH);
      }
        //myservo.write(target_speed_high);
      //Serial.println(target_speed_high);
    } else {
      if(enable_motor) {
        digitalWrite(ControllerPin, LOW);
      }
        //myservo.write(target_speed_low);
      //Serial.println(target_speed_low);
    }
  }

  // for debugging breathing back pressure sensing
//  current = analogRead(current_pin);
//  Serial.print(current);
//  Serial.print(",");
//  Serial.println(digitalRead(button_pin));

  loop_count += 1;
  delay(10);  // approximately 100 cycles per second
}
