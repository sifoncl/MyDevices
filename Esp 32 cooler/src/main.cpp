#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Coolix.h>
#include <PubSubClient.h>
#include <WiFi.h>

#define WIFI_SSID "DomWIFI"
#define WIFI_PASSWORD "bigdadwsad1996"
#define MQTT_SERVER "192.168.1.200"
#define MQTT_PORT 1883
#define MQTT_USER "device"
#define MQTT_PASSWORD "bigdadwsad1996"
#define IR_LED_PIN 23
#define IR_LED_INVERTED false
#define IR_SEND_REPEAT kCoolixDefaultRepeat

namespace DeviceConfig {
const char *kDeviceId = "esp32_coolix_remote";
const char *kDeviceName = "ESP32 Coolix Remote";
const char *kBaseTopic = "esp32-coolix/ac";
const char *kDiscoveryTopic = "homeassistant/climate/esp32_coolix_remote/config";
}  // namespace DeviceConfig

class ClimateRemote {
 public:
  virtual void begin() = 0;
  virtual void setPower(bool on) = 0;
  virtual void setMode(const String &mode) = 0;
  virtual void setTemperature(uint8_t temperature) = 0;
  virtual void setFanMode(const String &fanMode) = 0;
  virtual void setSwingMode(const String &swingMode) = 0;
  virtual void resend() = 0;
  virtual bool sendRaw48(const String &hexCode) = 0;
  virtual bool sendRaw24(const String &hexCode) = 0;
  virtual bool power() const = 0;
  virtual String mode() const = 0;
  virtual uint8_t temperature() const = 0;
  virtual String fanMode() const = 0;
  virtual String swingMode() const = 0;
  virtual String lastRaw24() const = 0;
  virtual String lastRaw48() const = 0;
};

class CoolixClimateRemote : public ClimateRemote {
 public:
  explicit CoolixClimateRemote(uint16_t irPin)
      : ac_(irPin, IR_LED_INVERTED), ir_(irPin, IR_LED_INVERTED) {}

  void begin() override {
    ac_.begin();
    ir_.begin();
    applyState();
  }

  void setPower(bool on) override {
    powerOn_ = on;
    applyState();
  }

  void setMode(const String &mode) override {
    if (mode == "off") {
      powerOn_ = false;
    } else if (mode == "auto" || mode == "cool" || mode == "heat" ||
               mode == "dry" || mode == "fan_only") {
      mode_ = mode;
      powerOn_ = true;
    }
    applyState();
  }

  void setTemperature(uint8_t temperature) override {
    temperature_ = constrain(temperature, 17, 30);
    if (powerOn_) {
      applyState();
    }
  }

  void setFanMode(const String &fanMode) override {
    if (fanMode == "auto" || fanMode == "low" || fanMode == "medium" ||
        fanMode == "high") {
      fanMode_ = fanMode;
      applyState();
    }
  }

  void setSwingMode(const String &swingMode) override {
    if (swingMode == "off" || swingMode == "vertical") {
      const bool nextSwing = swingMode == "vertical";
      if (nextSwing != swingOn_) {
        swingOn_ = nextSwing;
        sendCoolix48(kCoolixSwing);
      }
    }
  }

  void resend() override { applyState(); }

  bool sendRaw48(const String &hexCode) override {
    uint64_t raw48 = 0;
    if (!parseHex64(hexCode, &raw48) || raw48 > 0xFFFFFFFFFFFFULL) {
      return false;
    }

    lastRaw24_ = "";
    lastRaw48_ = toHex48(raw48);
    ir_.sendCoolix48(raw48, kCoolix48Bits, IR_SEND_REPEAT);
    logLastIr("raw48/direct");
    return true;
  }

  bool sendRaw24(const String &hexCode) override {
    uint64_t raw24 = 0;
    if (!parseHex64(hexCode, &raw24) || raw24 > 0xFFFFFFUL) {
      return false;
    }

    sendCoolix48(static_cast<uint32_t>(raw24));
    return true;
  }

  bool power() const override { return powerOn_; }
  String mode() const override { return powerOn_ ? mode_ : "off"; }
  uint8_t temperature() const override { return temperature_; }
  String fanMode() const override { return fanMode_; }
  String swingMode() const override { return swingOn_ ? "vertical" : "off"; }
  String lastRaw24() const override { return lastRaw24_; }
  String lastRaw48() const override { return lastRaw48_; }

 private:
  void applyState() {
    if (!powerOn_) {
      ac_.off();
      sendCoolix48(ac_.getRaw());
      return;
    }

    ac_.on();
    ac_.setMode(toCoolixMode(mode_));
    ac_.setTemp(temperature_);
    ac_.setFan(toCoolixFan(fanMode_));
    sendCoolix48(ac_.getRaw());
  }

  void sendCoolix48(uint32_t coolix24) {
    uint32_t inverted = coolix24 ^ 0xFFFFFF;
    uint64_t coolix48 = 0;

    coolix48 |= static_cast<uint64_t>((coolix24 >> 16) & 0xFF) << 40;
    coolix48 |= static_cast<uint64_t>((inverted >> 16) & 0xFF) << 32;
    coolix48 |= static_cast<uint64_t>((coolix24 >> 8) & 0xFF) << 24;
    coolix48 |= static_cast<uint64_t>((inverted >> 8) & 0xFF) << 16;
    coolix48 |= static_cast<uint64_t>(coolix24 & 0xFF) << 8;
    coolix48 |= static_cast<uint64_t>(inverted & 0xFF);

    lastRaw24_ = toHex24(coolix24);
    lastRaw48_ = toHex48(coolix48);
    ir_.sendCoolix48(coolix48, kCoolix48Bits, IR_SEND_REPEAT);
    logLastIr("coolix48/generated");
  }

  bool parseHex64(String text, uint64_t *value) const {
    text.trim();
    text.toLowerCase();
    if (text.startsWith("0x")) {
      text.remove(0, 2);
    }
    if (text.length() == 0 || text.length() > 12) {
      return false;
    }

    uint64_t result = 0;
    for (uint16_t i = 0; i < text.length(); i++) {
      char c = text.charAt(i);
      uint8_t nibble = 0;
      if (c >= '0' && c <= '9') {
        nibble = c - '0';
      } else if (c >= 'a' && c <= 'f') {
        nibble = c - 'a' + 10;
      } else {
        return false;
      }
      result = (result << 4) | nibble;
    }

    *value = result;
    return true;
  }

  String toHex24(uint32_t value) const {
    char buffer[9];
    snprintf(buffer, sizeof(buffer), "0x%06lX", static_cast<unsigned long>(value));
    return String(buffer);
  }

  String toHex48(uint64_t value) const {
    char buffer[15];
    snprintf(buffer, sizeof(buffer), "0x%04lX%08lX",
             static_cast<unsigned long>((value >> 32) & 0xFFFF),
             static_cast<unsigned long>(value & 0xFFFFFFFF));
    return String(buffer);
  }

  void logLastIr(const char *source) const {
    Serial.print("IR ");
    Serial.print(source);
    Serial.print(" raw24=");
    Serial.print(lastRaw24_);
    Serial.print(" raw48=");
    Serial.println(lastRaw48_);
  }

  uint8_t toCoolixMode(const String &mode) const {
    if (mode == "cool") return kCoolixCool;
    if (mode == "heat") return kCoolixHeat;
    if (mode == "dry") return kCoolixDry;
    if (mode == "fan_only") return kCoolixFan;
    return kCoolixAuto;
  }

  uint8_t toCoolixFan(const String &fanMode) const {
    if (fanMode == "low") return kCoolixFanMin;
    if (fanMode == "medium") return kCoolixFanMed;
    if (fanMode == "high") return kCoolixFanMax;
    return kCoolixFanAuto;
  }

  IRCoolixAC ac_;
  IRsend ir_;
  bool powerOn_ = false;
  bool swingOn_ = false;
  String mode_ = "cool";
  String fanMode_ = "auto";
  String lastRaw24_;
  String lastRaw48_;
  uint8_t temperature_ = 24;
};

class HomeAssistantMqttClimate {
 public:
  HomeAssistantMqttClimate(PubSubClient &mqtt, ClimateRemote &remote)
      : mqtt_(mqtt), remote_(remote) {}

  void begin() {
    mqtt_.setBufferSize(1536);
    publishDiscovery();
    publishAvailability(true);
    publishState();
    subscribeCommands();
  }

  void handleMessage(const String &topic, const String &payload) {
    if (topic == commandTopic("mode")) {
      remote_.setMode(payload);
    } else if (topic == commandTopic("temperature")) {
      remote_.setTemperature(payload.toInt());
    } else if (topic == commandTopic("fan_mode")) {
      remote_.setFanMode(payload);
    } else if (topic == commandTopic("swing_mode")) {
      remote_.setSwingMode(payload);
    } else if (topic == commandTopic("resend")) {
      remote_.resend();
    } else if (topic == commandTopic("raw48")) {
      if (!remote_.sendRaw48(payload)) {
        publishDebug("bad_raw48");
        return;
      }
    } else if (topic == commandTopic("raw24")) {
      if (!remote_.sendRaw24(payload)) {
        publishDebug("bad_raw24");
        return;
      }
    } else {
      return;
    }

    publishState();
  }

  void publishAvailability(bool online) {
    mqtt_.publish(topic("status").c_str(), online ? "online" : "offline", true);
  }

  void publishState() {
    String state = "{";
    state += "\"mode\":\"" + remote_.mode() + "\",";
    state += "\"temperature\":" + String(remote_.temperature()) + ",";
    state += "\"fan_mode\":\"" + remote_.fanMode() + "\",";
    state += "\"swing_mode\":\"" + remote_.swingMode() + "\",";
    state += "\"last_raw24\":\"" + remote_.lastRaw24() + "\",";
    state += "\"last_raw48\":\"" + remote_.lastRaw48() + "\"";
    state += "}";
    mqtt_.publish(topic("state").c_str(), state.c_str(), true);
  }

 private:
  String topic(const char *suffix) const {
    return String(DeviceConfig::kBaseTopic) + "/" + suffix;
  }

  String commandTopic(const char *command) const {
    return String(DeviceConfig::kBaseTopic) + "/set/" + command;
  }

  void subscribeCommands() {
    mqtt_.subscribe(commandTopic("mode").c_str());
    mqtt_.subscribe(commandTopic("temperature").c_str());
    mqtt_.subscribe(commandTopic("fan_mode").c_str());
    mqtt_.subscribe(commandTopic("swing_mode").c_str());
    mqtt_.subscribe(commandTopic("resend").c_str());
    mqtt_.subscribe(commandTopic("raw24").c_str());
    mqtt_.subscribe(commandTopic("raw48").c_str());
  }

  void publishDebug(const char *message) {
    mqtt_.publish(topic("debug").c_str(), message, false);
  }

  void publishDiscovery() {
    String cfg = "{";
    cfg += "\"name\":\"Coolix AC\",";
    cfg += "\"unique_id\":\"" + String(DeviceConfig::kDeviceId) + "\",";
    cfg += "\"object_id\":\"" + String(DeviceConfig::kDeviceId) + "\",";
    cfg += "\"availability_topic\":\"" + topic("status") + "\",";
    cfg += "\"payload_available\":\"online\",";
    cfg += "\"payload_not_available\":\"offline\",";
    cfg += "\"mode_command_topic\":\"" + commandTopic("mode") + "\",";
    cfg += "\"mode_state_topic\":\"" + topic("state") + "\",";
    cfg += "\"mode_state_template\":\"{{ value_json.mode }}\",";
    cfg += "\"temperature_command_topic\":\"" + commandTopic("temperature") + "\",";
    cfg += "\"temperature_state_topic\":\"" + topic("state") + "\",";
    cfg += "\"temperature_state_template\":\"{{ value_json.temperature }}\",";
    cfg += "\"fan_mode_command_topic\":\"" + commandTopic("fan_mode") + "\",";
    cfg += "\"fan_mode_state_topic\":\"" + topic("state") + "\",";
    cfg += "\"fan_mode_state_template\":\"{{ value_json.fan_mode }}\",";
    cfg += "\"swing_mode_command_topic\":\"" + commandTopic("swing_mode") + "\",";
    cfg += "\"swing_mode_state_topic\":\"" + topic("state") + "\",";
    cfg += "\"swing_mode_state_template\":\"{{ value_json.swing_mode }}\",";
    cfg += "\"min_temp\":17,";
    cfg += "\"max_temp\":30,";
    cfg += "\"temp_step\":1,";
    cfg += "\"precision\":1,";
    cfg += "\"modes\":[\"off\",\"auto\",\"cool\",\"heat\",\"dry\",\"fan_only\"],";
    cfg += "\"fan_modes\":[\"auto\",\"low\",\"medium\",\"high\"],";
    cfg += "\"swing_modes\":[\"off\",\"vertical\"],";
    cfg += "\"device\":{";
    cfg += "\"identifiers\":[\"" + String(DeviceConfig::kDeviceId) + "\"],";
    cfg += "\"name\":\"" + String(DeviceConfig::kDeviceName) + "\",";
    cfg += "\"manufacturer\":\"DIY\",";
    cfg += "\"model\":\"ESP32 Coolix MQTT IR Remote\"";
    cfg += "}";
    cfg += "}";

    mqtt_.publish(DeviceConfig::kDiscoveryTopic, cfg.c_str(), true);
  }

  PubSubClient &mqtt_;
  ClimateRemote &remote_;
};

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);
CoolixClimateRemote acRemote(IR_LED_PIN);
HomeAssistantMqttClimate haClimate(mqtt, acRemote);

void connectWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void mqttCallback(char *topic, byte *payload, unsigned int length) {
  String message;
  message.reserve(length);
  for (unsigned int i = 0; i < length; i++) {
    message += static_cast<char>(payload[i]);
  }
  message.trim();
  haClimate.handleMessage(String(topic), message);
}

void connectMqtt() {
  while (!mqtt.connected()) {
    String clientId = String(DeviceConfig::kDeviceId) + "-" + WiFi.macAddress();
    clientId.replace(":", "");

    if (mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD,
                     (String(DeviceConfig::kBaseTopic) + "/status").c_str(), 0,
                     true, "offline")) {
      haClimate.begin();
    } else {
      delay(3000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  acRemote.begin();
  connectWifi();
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(mqttCallback);
  connectMqtt();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWifi();
  }

  if (!mqtt.connected()) {
    connectMqtt();
  }

  mqtt.loop();
}
