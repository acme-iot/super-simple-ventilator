#include <Configuration.h>

#include <stdexcept>
#include <ArduinoJson.h>

AQUABOTICS_BEGIN_NAMESPACE

const String ConfigurationFile = "/config.json";

const String KeyId = "id";
const String KeyMqtt = "mqtt";
const String KeyMqttIp = "ip";
const String KeyMqttPort = "port";
const String KeyMqttUsername = "username";
const String KeyMqttPassword = "password";

Configuration::Configuration() {}

Configuration::Configuration(const Configuration &orig) {}

Configuration::~Configuration() {}

void Configuration::begin() {
  fs.begin();
}

/**
 * takes the configuration response string of valid JSON and writes/updates the configuration file
 *
 * @param json String that contains valid JSON
 * @throws std::runtime_error if the JSON string could not be deserialized
 */
void Configuration::load(String json) {
  Log.trace("begin::load(string)");
  DynamicJsonDocument doc(256);
  auto error = deserializeJson(doc, json.c_str());

  Log.trace("load(string):finished rehydrating json");

  if (error) {
    Log.trace("load(string):error rehydrating json");
    throw std::runtime_error(error.c_str());
  }

  Log.trace("load(string):begin cast document to JsonObject");
  auto jsonObject = doc.as<JsonObject>();
  Log.trace("load(string):finished cast document to JsonObject");

  // refresh settings
  Log.trace("load(string):begin refresh");
  try {
    this->refresh(jsonObject);
  } catch (std::exception& e) {
    Log.error("unable to refresh configuration, %s", e.what());
    return;
  }
  Log.trace("load(string):end refresh");

  // save response in config.json
  String cleanJson;
  serializeJson(doc, cleanJson);
  fs.writeFile(ConfigurationFile.c_str(), cleanJson.c_str());
}

void Configuration::load() {
  Log.trace("begin::load");
  if (!fs.existsFile(ConfigurationFile.c_str())) {
    return;
  }

  auto json = fs.readFile(ConfigurationFile.c_str());
  if (json) {
    load(*json);
  }
}

/**
 *
 * @param object JsonObject with MQTT properties
 * @throws std::runtime_error if any expected keys are missing
 */
void Configuration::refresh(JsonObject object) {
  Log.trace("begin::refresh");

  if (!object.containsKey(KeyId)) {
    throw std::runtime_error(("missing key " + KeyId).c_str());
  }

  if (!object.containsKey(KeyMqtt)) {
    throw std::runtime_error(("missing key " + KeyMqtt).c_str());
  }

  auto properties = object[KeyMqtt];

  if (!properties.containsKey(KeyMqttIp)) {
    throw std::runtime_error(("missing key " + KeyMqttIp).c_str());
  }

  if (!properties.containsKey(KeyMqttPort)) {
    throw std::runtime_error(("missing key " + KeyMqttPort).c_str());
  }

  if (!properties.containsKey(KeyMqttUsername)) {
    throw std::runtime_error(("missing key " + KeyMqttUsername).c_str());
  }

  if (!properties.containsKey(KeyMqttPassword)) {
    throw std::runtime_error(("missing key " + KeyMqttPassword).c_str());
  }

  auto ip = properties[KeyMqttIp];
  auto port = properties[KeyMqttPort];
  auto username = properties[KeyMqttUsername];
  auto password = properties[KeyMqttPassword];
  auto id = object[KeyId];

  setMqttIp(ip);
  setMqttPort(port);
  setMqttUsername(username);
  setMqttPassword(password);
  setId(id);

  initialized = true;
}

bool Configuration::hasInitialized() {
  Log.trace("begin::hasInitialized");
  return initialized;
}

bool Configuration::hasChanged(bool reset) {
  Log.trace("begin::hasChanged");
  auto temp = changed;

  if (reset) {
    changed = false;
  }

  return temp;
}

void Configuration::setId(String value) {
  Log.trace("begin::setMqttId");
  if (value.isEmpty()) {
    throw std::runtime_error("ID can't be empty");
  }

  if (id==value) {
    Log.trace("begin::setId value hasn't changed");
    return;
  }

  id = value;
  changed = true;
}

void Configuration::setMqttIp(String value) {
  Log.trace("begin::setMqttIp");
  if (value.isEmpty()) {
    throw std::runtime_error("MQTT IP can't be empty");
  }

  if (mqttIp==value) {
    Log.trace("begin::setMqttIp value hasn't changed");
    return;
  }

  mqttIp = value;
  changed = true;
}

void Configuration::setMqttPort(uint16_t value) {
  Log.trace("begin::setMqttPort");
  if (value==0) {
    throw std::runtime_error("MQTT port can't be empty");
  }

  if (mqttPort==value) {
    Log.trace("begin::setMqttPort value hasn't changed");
    return;
  }

  mqttPort = value;
  changed = true;
}

void Configuration::setMqttUsername(String value) {
  Log.trace("begin::setMqttUsername");

  if (mqttUsername==value) {
    Log.trace("begin::setMqttUsername value hasn't changed");
    return;
  }

  mqttUsername = value;
  changed = true;
}

void Configuration::setMqttPassword(String value) {
  Log.trace("begin::setMqttPassword");

  if (mqttPassword==value) {
    Log.trace("begin::setMqttPassword value hasn't changed");
    return;
  }

  mqttPassword = value;
  changed = true;
}

String Configuration::getId() {
  return id;
}

String Configuration::getMqttIp() {
  return mqttIp;
}

uint16_t Configuration::getMqttPort() {
  return mqttPort;
}

String Configuration::getMqttUsername() {
  return mqttUsername;
}

String Configuration::getMqttPassword() {
  return mqttPassword;
}

AQUABOTICS_END_NAMESPACE