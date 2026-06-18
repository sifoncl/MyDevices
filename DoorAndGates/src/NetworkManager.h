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
                 unsigned long connectTimeoutMs,
                 unsigned long reconnectIntervalMs);

  void begin();
  void loop();

  void saveCredentials(const String &ssid, const String &password);
  bool provisioningMode() const;
  bool connected() const;
  IPAddress address() const;
  String connectedSsid() const;
  const char *setupApSsid() const;

private:
  const char *_defaultSsid;
  const char *_defaultPassword;
  const char *_setupApSsid;
  const char *_setupApPassword;
  const char *_hostname;
  unsigned long _connectTimeoutMs;
  unsigned long _reconnectIntervalMs;

  Preferences _preferences;
  String _ssid;
  String _password;
  String _pendingSsid;
  String _pendingPassword;
  bool _apActive = false;
  bool _pendingConnect = false;
  bool _connectionAttemptActive = false;
  unsigned long _connectionAttemptAt = 0;
  unsigned long _lastReconnectAt = 0;

  void loadCredentials();
  void startStationConnection();
  void startProvisioningPortal();
  void finishStationConnection();
};

#endif
