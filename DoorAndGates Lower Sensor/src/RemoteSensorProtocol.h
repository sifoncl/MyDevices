#ifndef REMOTE_SENSOR_PROTOCOL_H
#define REMOTE_SENSOR_PROTOCOL_H

#include <Arduino.h>

constexpr uint32_t REMOTE_SENSOR_MAGIC = 0x44474D53UL;
constexpr uint8_t REMOTE_SENSOR_PROTOCOL_VERSION = 2;

enum class RemoteSensorRole : uint8_t
{
  Upper = 1,
  Lower = 2
};

enum class RemoteSensorTransport : uint8_t
{
  I2C = 1,
  SPI = 2
};

constexpr uint8_t REMOTE_SENSOR_FLAG_SENSOR_OK = 0x01;
constexpr uint8_t REMOTE_SENSOR_MOVING_X = 0x01;
constexpr uint8_t REMOTE_SENSOR_MOVING_Y = 0x02;
constexpr uint8_t REMOTE_SENSOR_MOVING_Z = 0x04;

#pragma pack(push, 1)
struct RemoteSensorPacket
{
  uint32_t magic;
  uint8_t version;
  uint8_t role;
  uint8_t transport;
  uint8_t flags;
  uint32_t sequence;
  uint32_t uptimeMs;
  float x;
  float y;
  float z;
  float velocityX;
  float velocityY;
  float velocityZ;
  int8_t directionX;
  int8_t directionY;
  int8_t directionZ;
  uint8_t movingAxes;
};
#pragma pack(pop)

static_assert(sizeof(RemoteSensorPacket) == 44, "ESP-NOW packet layout mismatch");

#endif
