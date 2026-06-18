#include "RemoteClimateSource.h"

#include "RemoteClimateProtocol.h"

#include <esp_idf_version.h>
#include <esp_now.h>
#include <string.h>

RemoteClimateSource *RemoteClimateSource::_instance = nullptr;

namespace
{
String payloadToString(const uint8_t *payload, unsigned int length)
{
  String value;
  value.reserve(length + 1);
  for (unsigned int i = 0; i < length; i++)
  {
    value += static_cast<char>(payload[i]);
  }
  value.trim();
  return value;
}

#if ESP_IDF_VERSION_MAJOR >= 5
void onEspNowReceive(const esp_now_recv_info_t *info,
                     const uint8_t *data,
                     int length)
{
  (void)info;
  RemoteClimateSource::dispatchEspNowPacket(data, length);
}
#else
void onEspNowReceive(const uint8_t *mac,
                     const uint8_t *data,
                     int length)
{
  (void)mac;
  RemoteClimateSource::dispatchEspNowPacket(data, length);
}
#endif
}

RemoteClimateSource::RemoteClimateSource(const char *clientId,
                                         const char *haTemperatureTopic,
                                         const char *haHumidityTopic,
                                         const char *mqttTemperatureTopic,
                                         const char *mqttHumidityTopic,
                                         const char *mqttValidTopic,
                                         uint32_t reconnectIntervalMs,
                                         uint32_t sampleTimeoutMs)
    : _clientId(clientId),
      _haTemperatureTopic(haTemperatureTopic),
      _haHumidityTopic(haHumidityTopic),
      _mqttTemperatureTopic(mqttTemperatureTopic),
      _mqttHumidityTopic(mqttHumidityTopic),
      _mqttValidTopic(mqttValidTopic),
      _reconnectIntervalMs(reconnectIntervalMs),
      _sampleTimeoutMs(sampleTimeoutMs),
      _mqtt(_client)
{
  _instance = this;
}

void RemoteClimateSource::begin(const char *mqttServer,
                                uint16_t mqttPort,
                                const char *mqttUser,
                                const char *mqttPassword)
{
  _client.setTimeout(1);
  _mqtt.setCallback(onMqttMessage);
  reconfigure(mqttServer, mqttPort, mqttUser, mqttPassword);
  beginEspNow();
}

bool RemoteClimateSource::reconfigure(const char *mqttServer,
                                      uint16_t mqttPort,
                                      const char *mqttUser,
                                      const char *mqttPassword)
{
  _server = mqttServer == nullptr ? "" : mqttServer;
  _port = mqttPort == 0 ? 1883 : mqttPort;
  _user = mqttUser == nullptr ? "" : mqttUser;
  _password = mqttPassword == nullptr ? "" : mqttPassword;
  _mqtt.disconnect();

  if (_server.isEmpty())
  {
    return false;
  }

  _mqtt.setServer(_server.c_str(), _port);
  _lastReconnectAt = 0;
  return true;
}

void RemoteClimateSource::loop()
{
  _mqtt.loop();

  if (_mqtt.connected() || _server.isEmpty() || WiFi.status() != WL_CONNECTED)
  {
    return;
  }

  const uint32_t now = millis();
  if (_lastReconnectAt != 0 && (now - _lastReconnectAt) < _reconnectIntervalMs)
  {
    return;
  }

  _lastReconnectAt = now;
  connectMqtt();
}

ClimateReading RemoteClimateSource::current(const TemperatureBoostSettings &boost) const
{
  if (sampleFresh(_haSample))
  {
    return readingFrom(_haSample, ClimateSourceKind::HomeAssistant, boost);
  }
  if (sampleFresh(_mqttSample))
  {
    return readingFrom(_mqttSample, ClimateSourceKind::Mqtt, boost);
  }
  if (sampleFresh(_espNowSample))
  {
    return readingFrom(_espNowSample, ClimateSourceKind::EspNow, boost);
  }

  return ClimateReading();
}

bool RemoteClimateSource::mqttConnected()
{
  return _mqtt.connected();
}

bool RemoteClimateSource::espNowReady() const
{
  return _espNowReady;
}

bool RemoteClimateSource::sourceFresh(ClimateSourceKind source) const
{
  switch (source)
  {
  case ClimateSourceKind::HomeAssistant:
    return sampleFresh(_haSample);
  case ClimateSourceKind::Mqtt:
    return sampleFresh(_mqttSample);
  case ClimateSourceKind::EspNow:
    return sampleFresh(_espNowSample);
  case ClimateSourceKind::None:
  default:
    return false;
  }
}

uint32_t RemoteClimateSource::sourceAgeMs(ClimateSourceKind source) const
{
  switch (source)
  {
  case ClimateSourceKind::HomeAssistant:
    return sampleAgeMs(_haSample);
  case ClimateSourceKind::Mqtt:
    return sampleAgeMs(_mqttSample);
  case ClimateSourceKind::EspNow:
    return sampleAgeMs(_espNowSample);
  case ClimateSourceKind::None:
  default:
    return 0;
  }
}

void RemoteClimateSource::dispatchEspNowPacket(const uint8_t *data, int length)
{
  if (_instance != nullptr)
  {
    _instance->handleEspNowPacket(data, length);
  }
}

void RemoteClimateSource::beginEspNow()
{
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("ESP-NOW climate receiver: init failed");
    _espNowReady = false;
    return;
  }

  esp_now_register_recv_cb(onEspNowReceive);
  _espNowReady = true;
  Serial.println("ESP-NOW climate receiver started");
}

void RemoteClimateSource::connectMqtt()
{
  if (_user.isEmpty())
  {
    _mqtt.connect(_clientId);
  }
  else
  {
    _mqtt.connect(_clientId, _user.c_str(), _password.c_str());
  }

  if (_mqtt.connected())
  {
    subscribeTopics();
  }
}

void RemoteClimateSource::subscribeTopics()
{
  _mqtt.subscribe(_haTemperatureTopic);
  _mqtt.subscribe(_haHumidityTopic);
  _mqtt.subscribe(_mqttTemperatureTopic);
  _mqtt.subscribe(_mqttHumidityTopic);
  _mqtt.subscribe(_mqttValidTopic);
}

void RemoteClimateSource::handleMqttMessage(char *topic,
                                            uint8_t *payload,
                                            unsigned int length)
{
  const String value = payloadToString(payload, length);
  const uint32_t now = millis();

  if (strcmp(topic, _haTemperatureTopic) == 0)
  {
    _haSample.temperature = value.toFloat();
    _haSample.temperatureSet = true;
    _haSample.validFlag = true;
    _haSample.updatedAt = now;
    _haSample.revision++;
    return;
  }

  if (strcmp(topic, _haHumidityTopic) == 0)
  {
    _haSample.humidity = value.toFloat();
    _haSample.humiditySet = true;
    _haSample.validFlag = true;
    _haSample.updatedAt = now;
    _haSample.revision++;
    return;
  }

  if (strcmp(topic, _mqttTemperatureTopic) == 0)
  {
    _mqttSample.temperature = value.toFloat();
    _mqttSample.temperatureSet = true;
    _mqttSample.updatedAt = now;
    _mqttSample.revision++;
    return;
  }

  if (strcmp(topic, _mqttHumidityTopic) == 0)
  {
    _mqttSample.humidity = value.toFloat();
    _mqttSample.humiditySet = true;
    _mqttSample.updatedAt = now;
    _mqttSample.revision++;
    return;
  }

  if (strcmp(topic, _mqttValidTopic) == 0)
  {
    _mqttSample.validFlag = value == "1" || value == "true" || value == "online";
    _mqttSample.updatedAt = now;
    _mqttSample.revision++;
  }
}

void RemoteClimateSource::handleEspNowPacket(const uint8_t *data, int length)
{
  if (length != static_cast<int>(sizeof(RemoteClimate::Packet)))
  {
    return;
  }

  RemoteClimate::Packet packet = {};
  memcpy(&packet, data, sizeof(packet));
  if (packet.magic != RemoteClimate::PACKET_MAGIC ||
      packet.version != RemoteClimate::PACKET_VERSION)
  {
    return;
  }

  _espNowSample.temperature = packet.temperature;
  _espNowSample.humidity = packet.humidity;
  _espNowSample.temperatureSet = packet.valid != 0;
  _espNowSample.humiditySet = packet.valid != 0;
  _espNowSample.validFlag = packet.valid != 0;
  _espNowSample.updatedAt = millis();
  _espNowSample.revision = packet.revision;
}

bool RemoteClimateSource::sampleFresh(const SourceSample &sample) const
{
  return sample.temperatureSet &&
         sample.humiditySet &&
         sample.validFlag &&
         sampleAgeMs(sample) <= _sampleTimeoutMs;
}

uint32_t RemoteClimateSource::sampleAgeMs(const SourceSample &sample) const
{
  if (sample.updatedAt == 0)
  {
    return UINT32_MAX;
  }
  return millis() - sample.updatedAt;
}

ClimateReading RemoteClimateSource::readingFrom(const SourceSample &sample,
                                                ClimateSourceKind source,
                                                const TemperatureBoostSettings &boost) const
{
  ClimateReading reading;
  reading.valid = true;
  reading.rawTemperature = sample.temperature;
  reading.displayTemperature = boost.apply(sample.temperature);
  reading.humidity = sample.humidity;
  reading.source = source;
  reading.ageMs = sampleAgeMs(sample);
  reading.revision = sample.revision;
  return reading;
}

void RemoteClimateSource::onMqttMessage(char *topic,
                                        uint8_t *payload,
                                        unsigned int length)
{
  if (_instance != nullptr)
  {
    _instance->handleMqttMessage(topic, payload, length);
  }
}
