#include "NetworkManager.h"

namespace
{
const IPAddress SETUP_AP_IP(192, 168, 4, 1);

const char *disconnectReasonText(uint8_t reason)
{
  switch (reason)
  {
  case WIFI_REASON_NO_AP_FOUND:
    return "network not found";
  case WIFI_REASON_AUTH_FAIL:
    return "authentication failed";
  case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
  case WIFI_REASON_HANDSHAKE_TIMEOUT:
    return "password/handshake failed";
  case WIFI_REASON_BEACON_TIMEOUT:
    return "signal lost";
  default:
    return "other";
  }
}

void logWiFiEvent(arduino_event_id_t event, arduino_event_info_t info)
{
  switch (event)
  {
  case ARDUINO_EVENT_WIFI_STA_START:
    Serial.println("WiFi event: station started");
    break;
  case ARDUINO_EVENT_WIFI_STA_CONNECTED:
    Serial.println("WiFi event: access point associated");
    break;
  case ARDUINO_EVENT_WIFI_STA_GOT_IP:
    Serial.print("WiFi event: IP ready, address ");
    Serial.println(IPAddress(info.got_ip.ip_info.ip.addr));
    break;
  case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
    Serial.print("WiFi event: disconnected, reason ");
    Serial.print(info.wifi_sta_disconnected.reason);
    Serial.print(" (");
    Serial.print(disconnectReasonText(info.wifi_sta_disconnected.reason));
    Serial.println(")");
    break;
  case ARDUINO_EVENT_WIFI_AP_START:
    Serial.println("WiFi event: setup access point started");
    break;
  default:
    break;
  }
}
}

NetworkManager::NetworkManager(const char *defaultSsid,
                               const char *defaultPassword,
                               const char *setupApSsid,
                               const char *setupApPassword,
                               const char *hostname,
                               unsigned long connectTimeoutMs,
                               unsigned long reconnectIntervalMs)
    : _defaultSsid(defaultSsid),
      _defaultPassword(defaultPassword),
      _setupApSsid(setupApSsid),
      _setupApPassword(setupApPassword),
      _hostname(hostname),
      _connectTimeoutMs(connectTimeoutMs),
      _reconnectIntervalMs(reconnectIntervalMs)
{
}

void NetworkManager::begin()
{
  Serial.println("Network: loading saved settings");
  loadSettings();
  Serial.print("Network: configured SSID is ");
  Serial.println(_ssid);

  WiFi.persistent(false);
  WiFi.setAutoReconnect(false);
  WiFi.setHostname(_hostname);
  WiFi.onEvent(logWiFiEvent);

  if (!WiFi.mode(WIFI_AP_STA))
  {
    Serial.println("Network error: cannot start WiFi AP+STA mode");
    return;
  }

  startSetupAccessPoint();
  startStationConnection();
}

void NetworkManager::loop()
{
  const unsigned long nowMs = millis();

  if (_pendingReconnect && (nowMs - _pendingReconnectAt) >= 750UL)
  {
    _pendingReconnect = false;
    startStationConnection();
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    _connectionAttemptActive = false;
    if (!_wasConnected)
    {
      _wasConnected = true;
      logConnected();
    }
    return;
  }

  if (_wasConnected)
  {
    _wasConnected = false;
    _lastReconnectAt = nowMs;
    Serial.println("WiFi disconnected");
  }

  if (_connectionAttemptActive)
  {
    if ((nowMs - _connectionAttemptAt) >= _connectTimeoutMs)
    {
      _connectionAttemptActive = false;
      _lastReconnectAt = nowMs;
      WiFi.disconnect(false, false);
      Serial.println("WiFi connection timed out; setup AP remains available");
    }
    return;
  }

  if (!_pendingReconnect && (nowMs - _lastReconnectAt) >= _reconnectIntervalMs)
  {
    startStationConnection();
  }
}

void NetworkManager::saveCredentials(const String &ssid, const String &password)
{
  saveSettings(ssid,
               password,
               false,
               IPAddress(),
               IPAddress(),
               IPAddress(),
               IPAddress(),
               IPAddress());
}

bool NetworkManager::saveSettings(const String &ssid,
                                  const String &password,
                                  bool useStaticIp,
                                  const IPAddress &localIp,
                                  const IPAddress &gateway,
                                  const IPAddress &subnet,
                                  const IPAddress &dns1,
                                  const IPAddress &dns2)
{
  if (ssid.isEmpty())
  {
    return false;
  }

  String newPassword = password;
  if (ssid == _ssid && password.isEmpty())
  {
    newPassword = _password;
  }

  if (!_preferences.begin("wifi_cfg", false))
  {
    return false;
  }

  _preferences.putString("ssid", ssid);
  _preferences.putString("password", newPassword);
  _preferences.putBool("static_ip", useStaticIp);
  writeIp(_preferences, "ip", localIp);
  writeIp(_preferences, "gateway", gateway);
  writeIp(_preferences, "subnet", subnet);
  writeIp(_preferences, "dns1", dns1);
  writeIp(_preferences, "dns2", dns2);
  _preferences.end();

  _ssid = ssid;
  _password = newPassword;
  _useStaticIp = useStaticIp;
  _localIp = localIp;
  _gateway = gateway;
  _subnet = subnet;
  _dns1 = dns1;
  _dns2 = dns2;
  _pendingReconnect = true;
  _pendingReconnectAt = millis();
  return true;
}

void NetworkManager::restoreDefaults()
{
  if (_preferences.begin("wifi_cfg", false))
  {
    _preferences.clear();
    _preferences.end();
  }

  _ssid = _defaultSsid == nullptr ? "" : _defaultSsid;
  _password = _defaultPassword == nullptr ? "" : _defaultPassword;
  _useStaticIp = false;
  _localIp = IPAddress();
  _gateway = IPAddress();
  _subnet = IPAddress();
  _dns1 = IPAddress();
  _dns2 = IPAddress();
  _pendingReconnect = true;
  _pendingReconnectAt = millis();
}

bool NetworkManager::provisioningMode() const
{
  return !connected();
}

bool NetworkManager::connected() const
{
  return WiFi.status() == WL_CONNECTED;
}

bool NetworkManager::staticIpEnabled() const
{
  return _useStaticIp;
}

IPAddress NetworkManager::address() const
{
  return connected() ? WiFi.localIP() : WiFi.softAPIP();
}

IPAddress NetworkManager::gateway() const
{
  return connected() ? WiFi.gatewayIP() : IPAddress();
}

IPAddress NetworkManager::subnet() const
{
  return connected() ? WiFi.subnetMask() : IPAddress();
}

IPAddress NetworkManager::dns1() const
{
  return connected() ? WiFi.dnsIP(0) : IPAddress();
}

IPAddress NetworkManager::dns2() const
{
  return connected() ? WiFi.dnsIP(1) : IPAddress();
}

IPAddress NetworkManager::configuredIp() const
{
  return _localIp;
}

IPAddress NetworkManager::configuredGateway() const
{
  return _gateway;
}

IPAddress NetworkManager::configuredSubnet() const
{
  return _subnet;
}

IPAddress NetworkManager::configuredDns1() const
{
  return _dns1;
}

IPAddress NetworkManager::configuredDns2() const
{
  return _dns2;
}

String NetworkManager::connectedSsid() const
{
  return connected() ? WiFi.SSID() : String();
}

String NetworkManager::configuredSsid() const
{
  return _ssid;
}

int32_t NetworkManager::rssi() const
{
  return connected() ? WiFi.RSSI() : 0;
}

const char *NetworkManager::setupApSsid() const
{
  return _setupApSsid;
}

void NetworkManager::loadSettings()
{
  _ssid = _defaultSsid == nullptr ? "" : _defaultSsid;
  _password = _defaultPassword == nullptr ? "" : _defaultPassword;
  _useStaticIp = false;
  _localIp = IPAddress();
  _gateway = IPAddress();
  _subnet = IPAddress();
  _dns1 = IPAddress();
  _dns2 = IPAddress();

  if (_preferences.begin("wifi_cfg", false))
  {
    if (_preferences.isKey("ssid"))
    {
      _ssid = _preferences.getString("ssid", _ssid);
    }
    if (_preferences.isKey("password"))
    {
      _password = _preferences.getString("password", _password);
    }
    if (_preferences.isKey("static_ip"))
    {
      _useStaticIp = _preferences.getBool("static_ip", false);
    }
    _localIp = readIp(_preferences, "ip");
    _gateway = readIp(_preferences, "gateway");
    _subnet = readIp(_preferences, "subnet");
    _dns1 = readIp(_preferences, "dns1");
    _dns2 = readIp(_preferences, "dns2");
    _preferences.end();
  }

  if (_useStaticIp &&
      (_localIp == IPAddress() || _gateway == IPAddress() || _subnet == IPAddress()))
  {
    _useStaticIp = false;
  }
}

void NetworkManager::startStationConnection()
{
  if (_ssid.isEmpty())
  {
    _lastReconnectAt = millis();
    return;
  }

  WiFi.disconnect(false, false);
  delay(20);
  applyIpConfiguration();
  const wl_status_t beginStatus = WiFi.begin(_ssid.c_str(), _password.c_str());

  _connectionAttemptAt = millis();
  _lastReconnectAt = _connectionAttemptAt;
  _connectionAttemptActive = true;

  Serial.print("Connecting to WiFi: ");
  Serial.print(_ssid);
  Serial.print(_useStaticIp ? " (static IP " : " (DHCP");
  if (_useStaticIp)
  {
    Serial.print(_localIp);
  }
  Serial.println(")");
  Serial.print("WiFi.begin status: ");
  Serial.println(static_cast<int>(beginStatus));
}

void NetworkManager::startSetupAccessPoint()
{
  const IPAddress subnet(255, 255, 255, 0);
  Serial.println("Network: configuring setup access point");
  const bool ipConfigured = WiFi.softAPConfig(SETUP_AP_IP, SETUP_AP_IP, subnet);
  Serial.print("Network: setup AP IP configuration ");
  Serial.println(ipConfigured ? "OK" : "FAILED");

  WiFi.softAPsetHostname(_hostname);
  const bool started = WiFi.softAP(_setupApSsid, _setupApPassword);

  Serial.print("Setup AP ");
  Serial.print(started ? "started: " : "failed: ");
  Serial.print(_setupApSsid);
  Serial.print(" at ");
  Serial.println(WiFi.softAPIP());
}

void NetworkManager::applyIpConfiguration()
{
  if (_useStaticIp)
  {
    WiFi.config(_localIp, _gateway, _subnet, _dns1, _dns2);
    return;
  }

  WiFi.config(IPAddress(), IPAddress(), IPAddress(), IPAddress(), IPAddress());
}

void NetworkManager::logConnected()
{
  Serial.print("WiFi connected to ");
  Serial.print(WiFi.SSID());
  Serial.print(", IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("ESP-NOW link AP channel: ");
  Serial.println(WiFi.channel());
}

IPAddress NetworkManager::readIp(Preferences &preferences, const char *key)
{
  IPAddress result;
  if (preferences.isKey(key))
  {
    result.fromString(preferences.getString(key, "0.0.0.0"));
  }
  return result;
}

void NetworkManager::writeIp(Preferences &preferences,
                             const char *key,
                             const IPAddress &value)
{
  preferences.putString(key, value.toString());
}
