#ifndef MQTT_SETTINGS_H
#define MQTT_SETTINGS_H

#include <Arduino.h>
#include <Preferences.h>

class MqttSettings
{
public:
  MqttSettings(const char *defaultServer,
               uint16_t defaultPort,
               const char *defaultUser,
               const char *defaultPassword);

  void begin();
  bool saveSettings(const String &server,
                    uint16_t port,
                    const String &user,
                    const String &password);
  void restoreDefaults();

  const String &server() const;
  uint16_t port() const;
  const String &user() const;
  const String &password() const;
  bool passwordSet() const;

private:
  const char *_defaultServer;
  uint16_t _defaultPort;
  const char *_defaultUser;
  const char *_defaultPassword;

  Preferences _preferences;
  String _server;
  uint16_t _port = 1883;
  String _user;
  String _password;

  void loadDefaults();
};

#endif
