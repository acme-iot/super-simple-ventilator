#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_ 1

#include <FileSystem.h>
#include <ArduinoJson.h>
#include <ArduinoLog.h>

AQUABOTICS_BEGIN_NAMESPACE

class Configuration {
 private:
  AQUABOTICS_NAMESPACE
  FileSystem fs;
  String mqttIp;
  String mqttUsername;
  String mqttPassword;
  uint16_t mqttPort;
  String id;
  bool changed=false;
  bool initialized=false;

  void refresh(JsonObject object);
  void setMqttIp(String value);
  void setMqttPort(uint16_t value);
  void setMqttUsername(String value);
  void setMqttPassword(String value);
  void setId(String value);

 public:
  Configuration();
  Configuration(const Configuration &orig);
  virtual ~Configuration();
  void begin();
  void load(String json);
  void load();
  bool hasChanged(bool reset = true);
  bool hasInitialized();
  String getMqttIp();
  uint16_t getMqttPort();
  String getMqttUsername();
  String getMqttPassword();
  String getId();
};

AQUABOTICS_END_NAMESPACE

#endif