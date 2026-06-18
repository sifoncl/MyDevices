#ifndef REMOTE_CLIMATE_PROTOCOL_H
#define REMOTE_CLIMATE_PROTOCOL_H

#include <Arduino.h>

namespace RemoteClimate
{
constexpr uint32_t PACKET_MAGIC = 0x42544D31UL; // BTM1
constexpr uint8_t PACKET_VERSION = 1;
constexpr size_t SOURCE_ID_SIZE = 24;

struct Packet
{
  uint32_t magic;
  uint8_t version;
  uint8_t valid;
  uint16_t reserved;
  float temperature;
  float humidity;
  uint32_t revision;
  uint32_t uptimeMs;
  char sourceId[SOURCE_ID_SIZE];
} __attribute__((packed));
}

#endif
