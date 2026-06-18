#include "MqttSettings.h"

namespace
{
constexpr char SETTINGS_NAMESPACE[] = "mqtt_cfg";
constexpr char KEY_SERVER[] = "server";
constexpr char KEY_PORT[] = "port";
constexpr char KEY_USER[] = "user";
constexpr char KEY_PASSWORD[] = "password";
}

MqttSettings::MqttSettings(const char *defaultServer,
                           uint16_t defaultPort,
                           const char *defaultUser,
                           const char *defaultPassword)
    : _defaultServer(defaultServer),
      _defaultPort(defaultPort),
      _defaultUser(defaultUser),
      _defaultPassword(defaultPassword)
{
}

void MqttSettings::begin()
{
  loadDefaults();

  if (_preferences.begin(SETTINGS_NAMESPACE, false))
  {
    _server = _preferences.getString(KEY_SERVER, _server);
    _port = _preferences.getUShort(KEY_PORT, _port);
    _user = _preferences.getString(KEY_USER, _user);
    _password = _preferences.getString(KEY_PASSWORD, _password);
    _preferences.end();
  }

  if (_server.isEmpty() || _port == 0)
  {
    loadDefaults();
  }
}

bool MqttSettings::saveSettings(const String &server,
                                uint16_t port,
                                const String &user,
                                const String &password)
{
  if (server.isEmpty() || port == 0)
  {
    return false;
  }

  if (!_preferences.begin(SETTINGS_NAMESPACE, false))
  {
    return false;
  }

  _preferences.putString(KEY_SERVER, server);
  _preferences.putUShort(KEY_PORT, port);
  _preferences.putString(KEY_USER, user);
  _preferences.putString(KEY_PASSWORD, password);
  _preferences.end();

  _server = server;
  _port = port;
  _user = user;
  _password = password;
  return true;
}

void MqttSettings::restoreDefaults()
{
  if (_preferences.begin(SETTINGS_NAMESPACE, false))
  {
    _preferences.clear();
    _preferences.end();
  }
  loadDefaults();
}

const String &MqttSettings::server() const
{
  return _server;
}

uint16_t MqttSettings::port() const
{
  return _port;
}

const String &MqttSettings::user() const
{
  return _user;
}

const String &MqttSettings::password() const
{
  return _password;
}

bool MqttSettings::passwordSet() const
{
  return !_password.isEmpty();
}

void MqttSettings::loadDefaults()
{
  _server = _defaultServer == nullptr ? "" : _defaultServer;
  _port = _defaultPort == 0 ? 1883 : _defaultPort;
  _user = _defaultUser == nullptr ? "" : _defaultUser;
  _password = _defaultPassword == nullptr ? "" : _defaultPassword;
}
