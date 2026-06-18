#ifndef SMARTHOMECONNECTOR_H // Защита от повторного включения
#define SMARTHOMECONNECTOR_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <functional>

class SmartHomeConnetctor
{
private:
    static const uint8_t maxSubscriptions = 16;

    // Настройки Wi-Fi
    const char *ssid;
    const char *password;

    // Настройки MQTT
    const char *mqtt_server;
    const int mqtt_port = 1883;
    const char *mqtt_user;
    const char *mqtt_password;
    // объект доступа к wifi
    WiFiClient espClient;

    // объект доступа к mqtt
    PubSubClient mqttClient;

    String subscriptions[maxSubscriptions];
    uint8_t subscriptionCount;
    std::function<void(char *, uint8_t *, unsigned int)> userCallback;

    void setupWiFi();   // Подключение к wifi
    void connectMQTT(); // Подключение/переподключение к mqtt
    void resubscribeTopics();
    bool rememberSubscription(const char *topic);

public:
    SmartHomeConnetctor(const char *wifi_ssid, const char *wifi_password,
                        const char *mqtt_srv, const char *mqtt_user,
                        const char *mqtt_pwd);
    // Методы для работы с MQTT
    void setup();
    void loop();

    // Методы для публикации и подписки
    bool publish(const char *topic, const char *message, bool retained = false);
    bool subscribe(const char *topic);

    // Установка callback-функции для обработки входящих сообщений
    void setCallback(MQTT_CALLBACK_SIGNATURE);

    // Проверка подключения к MQTT
    bool isConnected();
};
#endif // SMARTHOMECONNECTOR_H
