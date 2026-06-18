#include "WiFiUtils.h"

namespace
{
const char *savedSsid = nullptr;
const char *savedPassword = nullptr;
unsigned long reconnectIntervalMs = 10000UL;
unsigned long lastReconnectAttemptAt = 0;
bool reconnectConfigured = false;

void startConnectionAttempt()
{
  if (savedSsid == nullptr || savedPassword == nullptr)
  {
    return;
  }

  Serial.print("Подключение к ");
  Serial.println(savedSsid);
  WiFi.begin(savedSsid, savedPassword);
  lastReconnectAttemptAt = millis();
}
}

bool WiFiUtils::connect(const char *ssid, const char *password, uint8_t maxAttempts, unsigned long retryDelayMs, bool restartOnFail)
{
  if (ssid == nullptr || password == nullptr)
  {
    Serial.println("Не заданы параметры Wi-Fi");
    if (restartOnFail)
    {
      ESP.restart();
    }
    return false;
  }

  Serial.println();
  Serial.print("Подключение к ");
  Serial.println(ssid);
  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  beginAutoReconnect(ssid, password);

  WiFi.begin(ssid, password);
  lastReconnectAttemptAt = millis();
  delay(1000);

  uint8_t attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts)
  {
    delay(retryDelayMs);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("");
    Serial.println("Wi-Fi подключен");
    Serial.print("IP-адрес: ");
    Serial.println(WiFi.localIP());
    return true;
  }

  Serial.println("");
  Serial.println("Не удалось подключиться к Wi-Fi");
  if (restartOnFail)
  {
    ESP.restart();
  }
  return false;
}

bool WiFiUtils::setupWiFi(const char *ssid, const char *password, uint8_t maxAttempts, unsigned long retryDelayMs, bool restartOnFail)
{
  return connect(ssid, password, maxAttempts, retryDelayMs, restartOnFail);
}

void WiFiUtils::beginAutoReconnect(const char *ssid, const char *password, unsigned long retryIntervalMs)
{
  savedSsid = ssid;
  savedPassword = password;
  reconnectIntervalMs = retryIntervalMs < 1000UL ? 1000UL : retryIntervalMs;
  reconnectConfigured = savedSsid != nullptr && savedPassword != nullptr;
}

void WiFiUtils::loop()
{
  if (!reconnectConfigured || WiFi.status() == WL_CONNECTED)
  {
    return;
  }

  const unsigned long nowMs = millis();
  if ((nowMs - lastReconnectAttemptAt) < reconnectIntervalMs)
  {
    return;
  }

  Serial.println("Wi-Fi не подключен, повторная попытка...");
  startConnectionAttempt();
}

bool WiFiUtils::isConnected()
{
  return WiFi.status() == WL_CONNECTED;
}
