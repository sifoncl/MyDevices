#include "MagnetSensorController.h"

#include <string.h>

#include "AppConfig.h"

MagnetSensorController *MagnetSensorController::_instance = nullptr;

namespace
{
MagneticPresenceSensorConfig makeSensorConfig(float sensitivity)
{
  MagneticPresenceSensorConfig config;
  config.axis = MagneticPresenceAxis::X;
  config.sensitivity = sensitivity;
  config.motionThreshold = MAGNET_MOTION_THRESHOLD_UT;
  config.directionMinChange = MAGNET_DIRECTION_MIN_CHANGE_UT;
  config.settleTimeMs = MAGNET_SETTLE_TIME_MS;
  config.smoothingSamples = 1;
  return config;
}

struct RemoteMotionSample
{
  int8_t direction;
  bool moving;
  float velocity;
};

RemoteMotionSample selectRemoteMotion(const RemoteSensorPacket &packet,
                                      MagneticPresenceAxis axis)
{
  switch (axis)
  {
  case MagneticPresenceAxis::Y:
    return {packet.directionY,
            (packet.movingAxes & REMOTE_SENSOR_MOVING_Y) != 0,
            packet.velocityY};
  case MagneticPresenceAxis::Z:
    return {packet.directionZ,
            (packet.movingAxes & REMOTE_SENSOR_MOVING_Z) != 0,
            packet.velocityZ};
  case MagneticPresenceAxis::X:
  default:
    return {packet.directionX,
            (packet.movingAxes & REMOTE_SENSOR_MOVING_X) != 0,
            packet.velocityX};
  }
}
}

MagnetSensorController::MagnetSensorController(const uint8_t upperMac[6],
                                               const uint8_t lowerMac[6],
                                               unsigned long connectionTimeoutMs)
    : _upper(makeSensorConfig(DEFAULT_UPPER_SENSOR_SENSITIVITY)),
      _lower(makeSensorConfig(DEFAULT_LOWER_SENSOR_SENSITIVITY)),
      _connectionTimeoutMs(connectionTimeoutMs)
{
  memcpy(_upperSlot.expectedMac, upperMac, 6);
  memcpy(_lowerSlot.expectedMac, lowerMac, 6);
  _instance = this;
}

bool MagnetSensorController::begin()
{
  loadSettings();

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("ESP-NOW initialization failed");
    return false;
  }

  if (esp_now_register_recv_cb(onDataReceived) != ESP_OK)
  {
    Serial.println("ESP-NOW receive callback registration failed");
    return false;
  }

  Serial.println("ESP-NOW receiver ready");
  return true;
}

bool MagnetSensorController::loop()
{
  bool changed = false;
  changed = processSlot(_upperSlot, _upper) || changed;
  changed = processSlot(_lowerSlot, _lower) || changed;
  changed = updateTimeout(_upperSlot, _upper) || changed;
  changed = updateTimeout(_lowerSlot, _lower) || changed;
  return changed;
}

const MagneticPresenceSensorState &MagnetSensorController::upperState() const
{
  return _upper.state();
}

const MagneticPresenceSensorState &MagnetSensorController::lowerState() const
{
  return _lower.state();
}

const RemoteSensorLinkStatus &MagnetSensorController::upperLink() const
{
  return _upperSlot.link;
}

const RemoteSensorLinkStatus &MagnetSensorController::lowerLink() const
{
  return _lowerSlot.link;
}

void MagnetSensorController::setUpperSensitivity(float value)
{
  _upper.setSensitivity(value);
}

void MagnetSensorController::setLowerSensitivity(float value)
{
  _lower.setSensitivity(value);
}

void MagnetSensorController::setUpperAxis(MagneticPresenceAxis axis)
{
  if (_upper.axis() == axis)
  {
    return;
  }

  _upper.setAxis(axis);
  clearStoredBaseline("ubaseok");
}

void MagnetSensorController::setLowerAxis(MagneticPresenceAxis axis)
{
  if (_lower.axis() == axis)
  {
    return;
  }

  _lower.setAxis(axis);
  clearStoredBaseline("lbaseok");
}

void MagnetSensorController::setUpperInverted(bool inverted)
{
  _upper.setInvertDirection(inverted);
}

void MagnetSensorController::setLowerInverted(bool inverted)
{
  _lower.setInvertDirection(inverted);
}

bool MagnetSensorController::rememberUpperBaseline()
{
  if (!_upper.rememberCurrentAsBaseline())
  {
    return false;
  }
  saveBaseline("ubase", "ubaseok", _upper);
  return true;
}

bool MagnetSensorController::rememberLowerBaseline()
{
  if (!_lower.rememberCurrentAsBaseline())
  {
    return false;
  }
  saveBaseline("lbase", "lbaseok", _lower);
  return true;
}

void MagnetSensorController::saveSettings()
{
  if (!_preferences.begin("sensor_cfg", false))
  {
    return;
  }

  _preferences.putFloat("usens", _upper.sensitivity());
  _preferences.putFloat("lsens", _lower.sensitivity());
  _preferences.putUChar("uaxis", static_cast<uint8_t>(_upper.axis()));
  _preferences.putUChar("laxis", static_cast<uint8_t>(_lower.axis()));
  _preferences.putBool("uinvert", _upper.invertDirection());
  _preferences.putBool("linvert", _lower.invertDirection());
  _preferences.end();
}

const char *MagnetSensorController::axisToText(MagneticPresenceAxis axis)
{
  switch (axis)
  {
  case MagneticPresenceAxis::Y:
    return "y";
  case MagneticPresenceAxis::Z:
    return "z";
  case MagneticPresenceAxis::X:
  default:
    return "x";
  }
}

MagneticPresenceAxis MagnetSensorController::axisFromText(const String &text)
{
  if (text.equalsIgnoreCase("y"))
  {
    return MagneticPresenceAxis::Y;
  }
  if (text.equalsIgnoreCase("z"))
  {
    return MagneticPresenceAxis::Z;
  }
  return MagneticPresenceAxis::X;
}

const char *MagnetSensorController::transportToText(RemoteSensorTransport transport)
{
  return transport == RemoteSensorTransport::SPI ? "spi" : "i2c";
}

void MagnetSensorController::onDataReceived(const uint8_t *mac,
                                            const uint8_t *data,
                                            int length)
{
  if (_instance != nullptr)
  {
    _instance->handleReceivedPacket(mac, data, length);
  }
}

void MagnetSensorController::handleReceivedPacket(const uint8_t *mac,
                                                  const uint8_t *data,
                                                  int length)
{
  if (mac == nullptr || data == nullptr || length != static_cast<int>(sizeof(RemoteSensorPacket)))
  {
    return;
  }

  RemoteSensorPacket packet;
  memcpy(&packet, data, sizeof(packet));
  if (!packetValid(packet))
  {
    return;
  }

  SensorSlot *slot = nullptr;
  if (packet.role == static_cast<uint8_t>(RemoteSensorRole::Upper))
  {
    slot = &_upperSlot;
  }
  else if (packet.role == static_cast<uint8_t>(RemoteSensorRole::Lower))
  {
    slot = &_lowerSlot;
  }

  if (slot == nullptr || !macMatches(slot->expectedMac, mac))
  {
    return;
  }

  portENTER_CRITICAL(&_packetMux);
  slot->packet = packet;
  memcpy(slot->senderMac, mac, 6);
  slot->packetPending = true;
  portEXIT_CRITICAL(&_packetMux);
}

bool MagnetSensorController::processSlot(SensorSlot &slot, MagneticPresenceSensor &sensor)
{
  RemoteSensorPacket packet;
  uint8_t senderMac[6];
  bool pending = false;

  portENTER_CRITICAL(&_packetMux);
  if (slot.packetPending)
  {
    packet = slot.packet;
    memcpy(senderMac, slot.senderMac, 6);
    slot.packetPending = false;
    pending = true;
  }
  portEXIT_CRITICAL(&_packetMux);

  if (!pending)
  {
    return false;
  }

  const bool sensorOk = (packet.flags & REMOTE_SENSOR_FLAG_SENSOR_OK) != 0;
  if (sensorOk)
  {
    const RemoteMotionSample motion = selectRemoteMotion(packet, sensor.axis());
    sensor.updateRemote(packet.x,
                        packet.y,
                        packet.z,
                        motion.direction,
                        motion.moving,
                        motion.velocity);
  }
  else
  {
    sensor.markInvalid();
  }

  slot.link.connected = true;
  slot.link.sensorOk = sensorOk;
  memcpy(slot.link.mac, senderMac, 6);
  slot.link.sequence = packet.sequence;
  slot.link.packetsReceived++;
  slot.link.lastSeenAt = millis();
  slot.link.ageMs = 0;
  slot.link.transport = static_cast<RemoteSensorTransport>(packet.transport);
  return true;
}

bool MagnetSensorController::updateTimeout(SensorSlot &slot, MagneticPresenceSensor &sensor)
{
  if (!slot.link.connected)
  {
    return false;
  }

  slot.link.ageMs = millis() - slot.link.lastSeenAt;
  if (slot.link.ageMs <= _connectionTimeoutMs)
  {
    return false;
  }

  slot.link.connected = false;
  slot.link.sensorOk = false;
  sensor.markInvalid();
  return true;
}

bool MagnetSensorController::macIsUnset(const uint8_t mac[6])
{
  for (uint8_t i = 0; i < 6; i++)
  {
    if (mac[i] != 0)
    {
      return false;
    }
  }
  return true;
}

bool MagnetSensorController::macMatches(const uint8_t expected[6], const uint8_t actual[6])
{
  return macIsUnset(expected) || memcmp(expected, actual, 6) == 0;
}

bool MagnetSensorController::packetValid(const RemoteSensorPacket &packet)
{
  if (packet.magic != REMOTE_SENSOR_MAGIC ||
      packet.version != REMOTE_SENSOR_PROTOCOL_VERSION)
  {
    return false;
  }

  if (packet.role != static_cast<uint8_t>(RemoteSensorRole::Upper) &&
      packet.role != static_cast<uint8_t>(RemoteSensorRole::Lower))
  {
    return false;
  }

  if (packet.transport != static_cast<uint8_t>(RemoteSensorTransport::I2C) &&
      packet.transport != static_cast<uint8_t>(RemoteSensorTransport::SPI))
  {
    return false;
  }

  if (packet.directionX < -1 || packet.directionX > 1 ||
      packet.directionY < -1 || packet.directionY > 1 ||
      packet.directionZ < -1 || packet.directionZ > 1)
  {
    return false;
  }

  return (packet.movingAxes &
          ~(REMOTE_SENSOR_MOVING_X |
            REMOTE_SENSOR_MOVING_Y |
            REMOTE_SENSOR_MOVING_Z)) == 0;
}

void MagnetSensorController::loadSettings()
{
  if (!_preferences.begin("sensor_cfg", true))
  {
    return;
  }

  _upper.setSensitivity(_preferences.getFloat("usens", DEFAULT_UPPER_SENSOR_SENSITIVITY));
  _lower.setSensitivity(_preferences.getFloat("lsens", DEFAULT_LOWER_SENSOR_SENSITIVITY));

  const uint8_t upperAxis = min<uint8_t>(_preferences.getUChar("uaxis", 0), 2);
  const uint8_t lowerAxis = min<uint8_t>(_preferences.getUChar("laxis", 0), 2);
  _upper.setAxis(static_cast<MagneticPresenceAxis>(upperAxis));
  _lower.setAxis(static_cast<MagneticPresenceAxis>(lowerAxis));
  _upper.setInvertDirection(_preferences.getBool("uinvert", false));
  _lower.setInvertDirection(_preferences.getBool("linvert", false));

  if (_preferences.getBool("ubaseok", false))
  {
    _upper.setBaseline(_preferences.getFloat("ubase", 0.0f));
  }
  if (_preferences.getBool("lbaseok", false))
  {
    _lower.setBaseline(_preferences.getFloat("lbase", 0.0f));
  }

  _preferences.end();
}

void MagnetSensorController::saveBaseline(const char *valueKey,
                                          const char *validKey,
                                          MagneticPresenceSensor &sensor)
{
  if (!_preferences.begin("sensor_cfg", false))
  {
    return;
  }

  _preferences.putFloat(valueKey, sensor.baseline());
  _preferences.putBool(validKey, sensor.isCalibrated());
  _preferences.end();
}

void MagnetSensorController::clearStoredBaseline(const char *validKey)
{
  if (!_preferences.begin("sensor_cfg", false))
  {
    return;
  }

  _preferences.putBool(validKey, false);
  _preferences.end();
}
