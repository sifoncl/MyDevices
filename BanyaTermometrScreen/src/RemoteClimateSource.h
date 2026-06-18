#ifndef REMOTE_CLIMATE_SOURCE_H
#define REMOTE_CLIMATE_SOURCE_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>

#include "ClimateReading.h"
#include "TemperatureBoostSettings.h"

class RemoteClimateSource
{
public:
  RemoteClimateSource(const char *clientId,
                      const char *haTemperatureTopic,
                      const char *haHumidityTopic,
                      const char *mqttTemperatureTopic,
                      const char *mqttHumidityTopic,
                      const char *mqttValidTopic,
                      uint32_t reconnectIntervalMs,
                      uint32_t sampleTimeoutMs);

  void begin(const char *mqttServer,
             uint16_t mqttPort,
             const char *mqttUser,
             const char *mqttPassword);
  bool reconfigure(const char *mqttServer,
                   uint16_t mqttPort,
                   const char *mqttUser,
                   const char *mqttPassword);
  void loop();

  ClimateReading current(const TemperatureBoostSettings &boost) const;
  bool mqttConnected();
  bool espNowReady() const;
  bool sourceFresh(ClimateSourceKind source) const;
  uint32_t sourceAgeMs(ClimateSourceKind source) const;
  static void dispatchEspNowPacket(const uint8_t *data, int length);

private:
  struct SourceSample
  {
    bool temperatureSet = false;
    bool humiditySet = false;
    bool validFlag = true;
    float temperature = 0.0f;
    float humidity = 0.0f;
    uint32_t updatedAt = 0;
    uint32_t revision = 0;
  };

  static RemoteClimateSource *_instance;

  const char *_clientId;
  const char *_haTemperatureTopic;
  const char *_haHumidityTopic;
  const char *_mqttTemperatureTopic;
  const char *_mqttHumidityTopic;
  const char *_mqttValidTopic;
  uint32_t _reconnectIntervalMs;
  uint32_t _sampleTimeoutMs;

  WiFiClient _client;
  PubSubClient _mqtt;
  String _server;
  uint16_t _port = 1883;
  String _user;
  String _password;
  uint32_t _lastReconnectAt = 0;
  bool _espNowReady = false;

  SourceSample _haSample;
  SourceSample _mqttSample;
  SourceSample _espNowSample;

  void beginEspNow();
  void connectMqtt();
  void subscribeTopics();
  void handleMqttMessage(char *topic, uint8_t *payload, unsigned int length);
  void handleEspNowPacket(const uint8_t *data, int length);
  bool sampleFresh(const SourceSample &sample) const;
  uint32_t sampleAgeMs(const SourceSample &sample) const;
  ClimateReading readingFrom(const SourceSample &sample,
                             ClimateSourceKind source,
                             const TemperatureBoostSettings &boost) const;
  static void onMqttMessage(char *topic, uint8_t *payload, unsigned int length);
};

#endif
