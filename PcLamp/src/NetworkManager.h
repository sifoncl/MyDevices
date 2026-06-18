#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>

class NetworkManager
{
public:
  NetworkManager(const char *defaultSsid,
                 const char *defaultPassword,
                 const char *setupApSsid,
                 const char *setupApPassword,
                 const char *hostname,
                 uint32_t connectTimeoutMs,
                 uint32_t reconnectIntervalMs,
                 const IPAddress &setupApIp);

  void begin();
  void loop();

  bool saveSettings(const String &ssid,
                    const String &password,
                    bool useStaticIp,
                    const IPAddress &localIp,
                    const IPAddress &gateway,
                    const IPAddress &subnet,
                    const IPAddress &dns1,
                    const IPAddress &dns2);
  void restoreDefaults();

  bool connected() const;
  bool provisioningMode() const;
  bool staticIpEnabled() const;
  IPAddress address() const;
  IPAddress gateway() const;
  IPAddress subnet() const;
  IPAddress dns1() const;
  IPAddress dns2() const;
  IPAddress configuredIp() const;
  IPAddress configuredGateway() const;
  IPAddress configuredSubnet() const;
  IPAddress configuredDns1() const;
  IPAddress configuredDns2() const;
  String connectedSsid() const;
  String configuredSsid() const;
  int32_t rssi() const;
  const char *setupApSsid() const;

private:
  const char *_defaultSsid;
  const char *_defaultPassword;
  const char *_setupApSsid;
  const char *_setupApPassword;
  const char *_hostname;
  uint32_t _connectTimeoutMs;
  uint32_t _reconnectIntervalMs;
  IPAddress _setupApIp;

  Preferences _preferences;
  String _ssid;
  String _password;
  bool _useStaticIp = false;
  IPAddress _localIp;
  IPAddress _gateway;
  IPAddress _subnet;
  IPAddress _dns1;
  IPAddress _dns2;

  bool _connectionAttemptActive = false;
  bool _wasConnected = false;
  bool _pendingReconnect = false;
  uint32_t _connectionAttemptAt = 0;
  uint32_t _lastReconnectAt = 0;
  uint32_t _pendingReconnectAt = 0;

  void loadSettings();
  void startSetupAccessPoint();
  void startStationConnection();
  void applyIpConfiguration();
  void logConnected();
  static IPAddress readIp(Preferences &preferences, const char *key);
  static void writeIp(Preferences &preferences, const char *key, const IPAddress &value);
};

#endif
