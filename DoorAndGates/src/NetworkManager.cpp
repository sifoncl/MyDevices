#include "NetworkManager.h"

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
  loadCredentials();
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);
  WiFi.setHostname(_hostname);
  WiFi.mode(WIFI_AP_STA);
  startProvisioningPortal();
  startStationConnection();

  while (WiFi.status() != WL_CONNECTED &&
         (millis() - _connectionAttemptAt) < _connectTimeoutMs)
  {
    delay(250);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED)
  {
    finishStationConnection();
    return;
  }

  _connectionAttemptActive = false;
}

void NetworkManager::loop()
{
  const unsigned long nowMs = millis();

  if (_pendingConnect)
  {
    _pendingConnect = false;
    _ssid = _pendingSsid;
    _password = _pendingPassword;
    startStationConnection();
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    if (_connectionAttemptActive || _apActive)
    {
      finishStationConnection();
    }
    return;
  }

  if (_connectionAttemptActive)
  {
    if ((nowMs - _connectionAttemptAt) >= _connectTimeoutMs)
    {
      _connectionAttemptActive = false;
      startProvisioningPortal();
    }
    return;
  }

  if (!_apActive && (nowMs - _lastReconnectAt) >= _reconnectIntervalMs)
  {
    _lastReconnectAt = nowMs;
    startStationConnection();
  }
}

void NetworkManager::saveCredentials(const String &ssid, const String &password)
{
  if (ssid.length() == 0)
  {
    return;
  }

  if (_preferences.begin("wifi_cfg", false))
  {
    _preferences.putString("ssid", ssid);
    _preferences.putString("password", password);
    _preferences.end();
  }

  _pendingSsid = ssid;
  _pendingPassword = password;
  _pendingConnect = true;
}

bool NetworkManager::provisioningMode() const
{
  return !connected();
}

bool NetworkManager::connected() const
{
  return WiFi.status() == WL_CONNECTED;
}

IPAddress NetworkManager::address() const
{
  return connected() ? WiFi.localIP() : WiFi.softAPIP();
}

String NetworkManager::connectedSsid() const
{
  return connected() ? WiFi.SSID() : String();
}

const char *NetworkManager::setupApSsid() const
{
  return _setupApSsid;
}

void NetworkManager::loadCredentials()
{
  if (_preferences.begin("wifi_cfg", true))
  {
    _ssid = _preferences.getString("ssid", "");
    _password = _preferences.getString("password", "");
    _preferences.end();
  }

  if (_ssid.length() == 0)
  {
    _ssid = _defaultSsid == nullptr ? "" : _defaultSsid;
    _password = _defaultPassword == nullptr ? "" : _defaultPassword;
  }
}

void NetworkManager::startStationConnection()
{
  if (_ssid.length() == 0)
  {
    startProvisioningPortal();
    return;
  }

  WiFi.mode(WIFI_AP_STA);
  WiFi.disconnect(false, false);
  delay(50);
  WiFi.begin(_ssid.c_str(), _password.c_str());
  _connectionAttemptAt = millis();
  _lastReconnectAt = _connectionAttemptAt;
  _connectionAttemptActive = true;

  Serial.print("Connecting to WiFi: ");
  Serial.println(_ssid);
}

void NetworkManager::startProvisioningPortal()
{
  if (_apActive)
  {
    return;
  }

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPsetHostname(_hostname);
  _apActive = WiFi.softAP(_setupApSsid, _setupApPassword);

  Serial.print("WiFi setup AP: ");
  Serial.println(_setupApSsid);
  Serial.print("Setup IP: ");
  Serial.println(WiFi.softAPIP());
}

void NetworkManager::finishStationConnection()
{
  _connectionAttemptActive = false;

  Serial.print("WiFi connected: ");
  Serial.println(WiFi.localIP());
  Serial.print("ESP-NOW link AP channel: ");
  Serial.println(WiFi.channel());
}
