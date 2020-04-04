#include <Arduino.h>

#include <ArduinoLog.h>
#include <TimeLib.h>
#include "SSD1306Wire.h"

#include <chrono>
#include <ctime>
#include <string>

// #define RELEASE

#ifdef RELEASE
#define LOG_LEVEL LOG_LEVEL_SILENT
#define DEBUG false
#else
#define LOG_LEVEL LOG_LEVEL_VERBOSE
#define DEBUG true
#endif

const String Version = "0.1.1";

const uint8_t PotPin = A0;
const uint8_t ControllerInhalePin = D5;
const uint8_t ControllerExhalePin = D6;
const uint8_t ButtonPin = D7;
const uint8_t FlashPin = 0;

const uint8_t CycleLow = 6;
const uint8_t CycleHigh = 30;
const uint8_t CycleDelayMs = 10;

const uint8_t InhaleMode = 0;
const uint8_t ExhaleMode = 1;

const int BaudRate = 115200;
const uint8_t LineHeight = 12;

SSD1306Wire display(0x3c, SDA, SCL);

uint16_t cycleCount = 0;
uint8_t currentCycle = 0;
uint8_t cyclePhase = 0;

uint8_t breathingMode = InhaleMode;
uint8_t cycleInhale = 0;
uint8_t cycleExhale = 0;

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

  if (line >= LineHeight*4) {
    overwrite = true;
    line = LineHeight*2;
  }
}

void setupDisplay(String message) {
  display.init();
  display.flipScreenVertically();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, message);
  display.display();
  line += LineHeight;
}

void preInitialize() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(FlashPin, INPUT);
  pinMode(PotPin, INPUT);
  pinMode(ButtonPin, INPUT);
  pinMode(ControllerInhalePin, OUTPUT);
  pinMode(ControllerExhalePin, OUTPUT);

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

void refreshDisplay() {
  const uint8_t Padding = 5;
  line = LineHeight + Padding;
  pout("Mode " + String(breathingMode==InhaleMode ? "Inhale" : "Exhale"));
  pout("  Inhale " + String(cycleInhale) + "/min");
  pout("  Exhale " + String(cycleExhale) + "/min");
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

void setupController() {
  digitalWrite(ControllerInhalePin, LOW);
  digitalWrite(ControllerExhalePin, LOW);
}

uint8_t getPotValue() {
  const uint16_t potValue = analogRead(A0);
  return map(potValue, 0, 1024, CycleLow, CycleHigh);
}

void initialize() {
  const uint8_t potValue = getPotValue();
  cycleInhale = cycleExhale = potValue;
}

void setup() {
  preInitialize();
  setupDebugging();
  setupLogging();
  printBoardInfo();
  initialize();
  setupController();
  setupDisplay("Ventilator " + Version);
  delay(3000);
  refreshDisplay();

  Log.notice("Running...");

  digitalWrite(LED_BUILTIN, HIGH);
}

uint16_t breathIn = 0;
uint16_t breathOut = 0;
bool buttonPushed = false;

void loop() {

  // handle button
  auto buttonState = digitalRead(ButtonPin);
  if (buttonPushed && buttonState==LOW) {
    // button released
    buttonPushed = false;
    breathingMode = !breathingMode;
    refreshDisplay();
    Log.trace("Mode %d", breathingMode);
  }

  if (!buttonPushed && buttonState==HIGH) {
    buttonPushed = true;
  }

  const uint8_t normailizedRate = getPotValue();

  auto *ptrCycle = breathingMode==InhaleMode ? &cycleInhale : &cycleExhale;

  if (normailizedRate!=*ptrCycle) {
    *ptrCycle = normailizedRate;
    Log.trace("Setting cycle to %d", *ptrCycle);
    refreshDisplay();
  }

  cycleCount++;
  currentCycle = cyclePhase ? cycleInhale : cycleExhale;
  if ((30*100)/currentCycle < cycleCount) {
    cyclePhase = (cyclePhase + 1)%2;
    cycleCount = 0;

    if (cyclePhase) {
      if (DEBUG) {
        Log.trace("Breath out %dms", breathOut);
        breathIn = 0;
      } else {
        digitalWrite(ControllerExhalePin, LOW);
        digitalWrite(ControllerInhalePin, HIGH);
      }
    } else {
      if (DEBUG) {
        Log.trace("Breath in %dms", breathIn);
        breathOut = 0;
      } else {
        digitalWrite(ControllerExhalePin, HIGH);
        digitalWrite(ControllerInhalePin, LOW);
      }
    }
  }

  breathIn += CycleDelayMs;
  breathOut += CycleDelayMs;
  delay(CycleDelayMs);  // approximately 100 cycles per second
}
