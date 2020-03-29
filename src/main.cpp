#include <Arduino.h>

#include <ArduinoLog.h>
#include <TimeLib.h>
#include "SSD1306Wire.h"

#include <chrono>
#include <ctime>
#include <string>

#ifdef RELEASE
#define LOG_LEVEL LOG_LEVEL_SILENT
#define DEBUG false
#else
#define LOG_LEVEL LOG_LEVEL_VERBOSE
#define DEBUG true
#endif

const String Version = "0.0.1";

const uint8_t PotPin = A0;
const uint8_t ControllerPin = D5;
const uint8_t FlashPin = 0;

const uint8_t CycleLow = 13;
const uint8_t CycleHigh = 21;
const uint8_t CycleDelayMs = 10;

const int BaudRate = 115200;
const uint8_t LineHeight = 16;

SSD1306Wire display(0x3c, SDA, SCL);

uint16_t cycleCount = 0;
uint8_t currentCycle = 0;
uint8_t cyclePhase = 0;
uint8_t line = LineHeight;
bool overwrite = false;

void pout(String msg) {
  if (overwrite) {
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

  if (line >= LineHeight*3) {
    overwrite = true;
    line = LineHeight*2;
  }
}

void preInitialize() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(FlashPin, INPUT);
  pinMode(PotPin, INPUT);
  pinMode(ControllerPin, OUTPUT);

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

void setupController() {
  digitalWrite(ControllerPin, LOW);
}

void setup() {
  preInitialize();
  setupDebugging();
  setupLogging();
  printBoardInfo();
  setupDisplay();
  setupController();

  Log.notice("Running...");

  digitalWrite(LED_BUILTIN, HIGH);
}

int breathIn = 0;
int breathOut = 0;
void loop() {
  const uint16_t potValue = analogRead(A0);
  const uint8_t normailizedRate = map(potValue, 0, 1024, CycleLow, CycleHigh);

  if (normailizedRate!=currentCycle) {
    currentCycle = normailizedRate;
    pout("Cycles " + String(currentCycle) + "/min");
  }

  cycleCount += 1;
  if ((30*100)/currentCycle < cycleCount) {
    cyclePhase = (cyclePhase + 1)%2;
    cycleCount = 0;

    if (cyclePhase==0) {
      if (DEBUG) {
        Log.trace("Breath out %dms", breathOut);
        breathIn = 0;
      } else {
        digitalWrite(ControllerPin, HIGH);
      }
    } else {
      if (DEBUG) {
        Log.trace("Breath in %dms", breathIn);
        breathOut = 0;
      } else {
        //todo, reverse airflow
        digitalWrite(ControllerPin, LOW);
      }
    }
  }

  breathIn += CycleDelayMs;
  breathOut += CycleDelayMs;
  delay(CycleDelayMs);  // approximately 100 cycles per second
}
