#include "EspNowClimateBroadcaster.h"

#include "AppConfig.h"
#include "RemoteClimateProtocol.h"

#include <WiFi.h>
#include <esp_now.h>
#include <string.h>

namespace
{
const uint8_t BROADCAST_MAC[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
}

EspNowClimateBroadcaster::EspNowClimateBroadcaster(uint32_t publishIntervalMs)
    : _publishIntervalMs(publishIntervalMs)
{
}

void EspNowClimateBroadcaster::begin()
{
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("ESP-NOW climate broadcaster: init failed");
    _ready = false;
    return;
  }

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, BROADCAST_MAC, sizeof(BROADCAST_MAC));
  peer.channel = 0;
  peer.encrypt = false;

  if (!esp_now_is_peer_exist(BROADCAST_MAC))
  {
    const esp_err_t result = esp_now_add_peer(&peer);
    if (result != ESP_OK)
    {
      Serial.print("ESP-NOW climate broadcaster: add peer failed ");
      Serial.println(static_cast<int>(result));
      _ready = false;
      return;
    }
  }

  _ready = true;
  Serial.println("ESP-NOW climate broadcaster started");
}

void EspNowClimateBroadcaster::loop(const ClimateSensor &climate)
{
  if (!_ready)
  {
    return;
  }

  const uint32_t now = millis();
  const bool changed = _lastRevision != climate.revision();
  const bool periodic = (now - _lastPublishAt) >= _publishIntervalMs;
  if (!changed && !periodic)
  {
    return;
  }

  _lastRevision = climate.revision();
  _lastPublishAt = now;
  publish(climate);
}

bool EspNowClimateBroadcaster::ready() const
{
  return _ready;
}

void EspNowClimateBroadcaster::publish(const ClimateSensor &climate)
{
  RemoteClimate::Packet packet = {};
  packet.magic = RemoteClimate::PACKET_MAGIC;
  packet.version = RemoteClimate::PACKET_VERSION;
  packet.valid = climate.valid() ? 1 : 0;
  packet.temperature = climate.valid() ? climate.temperature() : 0.0f;
  packet.humidity = climate.valid() ? climate.humidity() : 0.0f;
  packet.revision = climate.revision();
  packet.uptimeMs = millis();
  strlcpy(packet.sourceId, Config::DEVICE_ID, sizeof(packet.sourceId));

  esp_now_send(BROADCAST_MAC,
               reinterpret_cast<const uint8_t *>(&packet),
               sizeof(packet));
}
