#ifndef MAGNET_SENSOR_CONTROLLER_H
#define MAGNET_SENSOR_CONTROLLER_H

#include <MagneticPresenceSensor.h>
#include <Preferences.h>
#include <esp_now.h>

#include "RemoteSensorProtocol.h"

struct RemoteSensorLinkStatus
{
  bool connected = false;
  bool sensorOk = false;
  uint8_t mac[6] = {0};
  uint32_t sequence = 0;
  uint32_t packetsReceived = 0;
  unsigned long lastSeenAt = 0;
  unsigned long ageMs = 0;
  RemoteSensorTransport transport = RemoteSensorTransport::I2C;
};

class MagnetSensorController
{
public:
  MagnetSensorController(const uint8_t upperMac[6],
                         const uint8_t lowerMac[6],
                         unsigned long connectionTimeoutMs);

  bool begin();
  bool loop();

  const MagneticPresenceSensorState &upperState() const;
  const MagneticPresenceSensorState &lowerState() const;
  const RemoteSensorLinkStatus &upperLink() const;
  const RemoteSensorLinkStatus &lowerLink() const;

  void setUpperSensitivity(float value);
  void setLowerSensitivity(float value);
  void setUpperAxis(MagneticPresenceAxis axis);
  void setLowerAxis(MagneticPresenceAxis axis);
  void setUpperInverted(bool inverted);
  void setLowerInverted(bool inverted);
  bool rememberUpperBaseline();
  bool rememberLowerBaseline();
  void saveSettings();

  static const char *axisToText(MagneticPresenceAxis axis);
  static MagneticPresenceAxis axisFromText(const String &text);
  static const char *transportToText(RemoteSensorTransport transport);

private:
  struct SensorSlot
  {
    uint8_t expectedMac[6] = {0};
    bool packetPending = false;
    RemoteSensorPacket packet = {};
    uint8_t senderMac[6] = {0};
    RemoteSensorLinkStatus link;
  };

  static MagnetSensorController *_instance;

  MagneticPresenceSensor _upper;
  MagneticPresenceSensor _lower;
  Preferences _preferences;
  SensorSlot _upperSlot;
  SensorSlot _lowerSlot;
  unsigned long _connectionTimeoutMs;
  portMUX_TYPE _packetMux = portMUX_INITIALIZER_UNLOCKED;

  static void onDataReceived(const uint8_t *mac, const uint8_t *data, int length);
  void handleReceivedPacket(const uint8_t *mac, const uint8_t *data, int length);
  bool processSlot(SensorSlot &slot, MagneticPresenceSensor &sensor);
  bool updateTimeout(SensorSlot &slot, MagneticPresenceSensor &sensor);
  static bool macIsUnset(const uint8_t mac[6]);
  static bool macMatches(const uint8_t expected[6], const uint8_t actual[6]);
  static bool packetValid(const RemoteSensorPacket &packet);

  void loadSettings();
  void saveBaseline(const char *valueKey, const char *validKey, MagneticPresenceSensor &sensor);
  void clearStoredBaseline(const char *validKey);
};

#endif
