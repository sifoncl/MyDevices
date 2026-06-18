#include "SmartHomeConnetctor.h"

namespace
{
const unsigned int logPayloadLimit = 120;

void printMessagePreview(const char *message)
{
    Serial.print(" payload=");

    unsigned int i = 0;
    while (message[i] != '\0' && i < logPayloadLimit)
    {
        char c = message[i];
        Serial.print((c >= 32 && c <= 126) ? c : '.');
        i++;
    }

    if (message[i] != '\0')
    {
        Serial.print("...");
    }
}

void printPayloadPreview(const uint8_t *payload, unsigned int length)
{
    Serial.print(" payload=");

    unsigned int previewLength = length < logPayloadLimit ? length : logPayloadLimit;
    for (unsigned int i = 0; i < previewLength; i++)
    {
        char c = static_cast<char>(payload[i]);
        Serial.print((c >= 32 && c <= 126) ? c : '.');
    }

    if (length > logPayloadLimit)
    {
        Serial.print("...");
    }
}
}

SmartHomeConnetctor::SmartHomeConnetctor(const char *wifi_ssid, const char *wifi_password,
                                         const char *mqtt_srv, const char *mqtt_usr,
                                         const char *mqtt_pwd)
    : ssid(wifi_ssid),
      password(wifi_password),
      mqtt_server(mqtt_srv),
      mqtt_user(mqtt_usr),
      mqtt_password(mqtt_pwd), // Инициализируем правильный член класса
      subscriptionCount(0)
{
    Serial.println("[MQTT] Connector init");

    setupWiFi();
    mqttClient.setClient(espClient);

    // Настраиваем MQTT
    mqttClient.setServer(mqtt_server, mqtt_port);
    mqttClient.setBufferSize(1024);
    // Устанавливаем таймауты для более стабильного подключения
    mqttClient.setSocketTimeout(30);
    mqttClient.setKeepAlive(60);

    // Принудительно подключаемся к MQTT
    connectMQTT();
}

void SmartHomeConnetctor::setupWiFi()
{
    Serial.println();
    Serial.print("[WiFi] Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20)
    {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("");
        Serial.println("[WiFi] Connected");
        Serial.print("[WiFi] IP: ");
        Serial.println(WiFi.localIP());
        Serial.print("[WiFi] RSSI: ");
        Serial.println(WiFi.RSSI());
    }
    else
    {
        Serial.println("");
        Serial.println("[WiFi] Connection failed, restarting");
        ESP.restart();
    }
}

void SmartHomeConnetctor::connectMQTT()
{
    // Проверяем подключение к Wi-Fi
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("[WiFi] Connection lost, reconnecting");
        setupWiFi();
    }

    // Пытаемся подключиться к MQTT
    if (!mqttClient.connected())
    {
        String clientId = "ESP32_";
        clientId += String((uint32_t)ESP.getEfuseMac(), HEX);

        Serial.print("[MQTT] Connecting to ");
        Serial.print(mqtt_server);
        Serial.print(":");
        Serial.print(mqtt_port);
        Serial.print(" as ");
        Serial.println(clientId);

        if (mqttClient.connect(clientId.c_str(), mqtt_user, mqtt_password))
        {
            Serial.println("[MQTT] Connected");
            resubscribeTopics();
        }
        else
        {
            Serial.print("[MQTT] Connection failed, state=");
            Serial.print(mqttClient.state());
            Serial.println("; retry in 5 seconds");
            delay(5000);
        }
    }
}

bool SmartHomeConnetctor::publish(const char *topic, const char *message, bool retained)
{
    if (topic == nullptr || message == nullptr)
    {
        Serial.println("[MQTT] Publish failed: null topic or message");
        return false;
    }

    if (!mqttClient.connected())
    {
        Serial.print("[MQTT] Publish requested while disconnected, topic=");
        Serial.println(topic);
        connectMQTT();
    }

    if (mqttClient.connected())
    {
        bool ok = mqttClient.publish(topic, message, retained);
        Serial.print("[MQTT] -> ");
        Serial.print(topic);
        Serial.print(" retained=");
        Serial.print(retained ? "true" : "false");
        Serial.print(" result=");
        Serial.print(ok ? "ok" : "failed");
        printMessagePreview(message);
        Serial.println();
        return ok;
    }

    Serial.print("[MQTT] Publish skipped, still disconnected, topic=");
    Serial.println(topic);
    return false;
}

void SmartHomeConnetctor::loop()
{
    // Поддерживаем соединение с MQTT
    if (!mqttClient.connected())
    {
        connectMQTT();
    }

    // Обрабатываем входящие сообщения
    mqttClient.loop();
}

bool SmartHomeConnetctor::subscribe(const char *topic)
{
    if (topic == nullptr)
    {
        Serial.println("[MQTT] Subscribe failed: null topic");
        return false;
    }

    if (topic[0] == '\0')
    {
        Serial.println("[MQTT] Subscribe failed: empty topic");
        return false;
    }

    rememberSubscription(topic);

    if (!mqttClient.connected())
    {
        Serial.print("[MQTT] Subscribe requested while disconnected, topic=");
        Serial.println(topic);
        connectMQTT();
    }

    if (mqttClient.connected())
    {
        bool ok = mqttClient.subscribe(topic);
        Serial.print("[MQTT] Subscribe ");
        Serial.print(topic);
        Serial.print(" result=");
        Serial.println(ok ? "ok" : "failed");
        return ok;
    }

    Serial.print("[MQTT] Subscribe deferred, topic=");
    Serial.println(topic);
    return false;
}

void SmartHomeConnetctor::setCallback(MQTT_CALLBACK_SIGNATURE)
{
    userCallback = callback;
    mqttClient.setCallback([this](char *topic, uint8_t *payload, unsigned int length)
                           {
                               Serial.print("[MQTT] <- ");
                               Serial.print(topic);
                               Serial.print(" bytes=");
                               Serial.print(length);
                               printPayloadPreview(payload, length);
                               Serial.println();

                               if (userCallback)
                               {
                                   userCallback(topic, payload, length);
                               }
                           });
    Serial.println("[MQTT] Callback configured");
}

bool SmartHomeConnetctor::isConnected()
{
    return mqttClient.connected();
}

void SmartHomeConnetctor::setup()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        setupWiFi();
    }
    connectMQTT();
}

bool SmartHomeConnetctor::rememberSubscription(const char *topic)
{
    for (uint8_t i = 0; i < subscriptionCount; i++)
    {
        if (subscriptions[i] == topic)
        {
            return true;
        }
    }

    if (subscriptionCount >= maxSubscriptions)
    {
        Serial.print("[MQTT] Subscription list full, topic not remembered: ");
        Serial.println(topic);
        return false;
    }

    subscriptions[subscriptionCount] = String(topic);
    subscriptionCount++;
    return true;
}

void SmartHomeConnetctor::resubscribeTopics()
{
    for (uint8_t i = 0; i < subscriptionCount; i++)
    {
        const char *topic = subscriptions[i].c_str();
        if (topic == nullptr || topic[0] == '\0')
        {
            continue;
        }

        bool ok = mqttClient.subscribe(topic);
        Serial.print("[MQTT] Resubscribe ");
        Serial.print(topic);
        Serial.print(" result=");
        Serial.println(ok ? "ok" : "failed");
    }
}
