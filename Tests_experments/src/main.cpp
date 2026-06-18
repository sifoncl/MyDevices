// #include "RGBWWWController.h"
// #include "SmartHomeConnetctor.h"

// #define RED_PIN 21
// #define GREEN_PIN 22
// #define BLUE_PIN 19
// #define WARM_WHITE_PIN 23
// #define COLD_WHITE_PIN 18

// RGBWWWController *stripController = nullptr;
// SmartHomeConnetctor *connector = nullptr;

// // Конфигурационные данные
// const char *WIFI_SSID = "DomWIFI";
// const char *WIFI_PASSWORD = "bigdadwsad1996";
// const char *MQTT_SERVER = "192.168.1.100";
// const char *MQTT_USER = "device";
// const char *MQTT_PASSWORD = "bigdadwsad1996";

// // Уникальные идентификаторы устройств
// const char *rgb_device_id = "rgbwwcw_light_01";
// const char *rgb_device_name = "Лампа над Пк 1";

// // Топики MQTT для RGB устройства
// const char *rgb_discovery_topic = "homeassistant/light/rgbwwcw_light_01/config";
// const char *rgb_state_topic = "home/light/rgbwwcw_light_01/state";
// const char *rgb_command_topic = "home/light/rgbwwcw_light_01/set";
// const char *rgb_availability_topic = "home/light/rgbwwcw_light_01/availability";

// void callBack(char *topic, byte *payload, unsigned int length)
// {
//     Serial.println("Зашел в callback");
//     stripController->processMqttCallBack(topic, payload, length);
// }

// void setup()
// {
//     Serial.begin(115200);
//     Serial.println("Starting...");

//     stripController = new RGBWWWController(RED_PIN, GREEN_PIN, BLUE_PIN, COLD_WHITE_PIN, WARM_WHITE_PIN);
//     connector = new SmartHomeConnetctor(WIFI_SSID, WIFI_PASSWORD,
//                                         MQTT_SERVER, MQTT_USER, MQTT_PASSWORD);
//     stripController->setRGBWW(255, 255, 255, 255, 255, 255);
//     stripController->setMqttParams(rgb_device_id, rgb_device_name, rgb_command_topic, rgb_state_topic);

//     connector->publish(rgb_discovery_topic, stripController->getDiscovery().c_str());
//     connector->publish(rgb_state_topic, stripController->getStateJson().c_str());
//     connector->setCallback(callBack);
//     connector->subscribe(rgb_command_topic);
// }
// void loop()
// {
//     connector->loop();
//     delay(100);
// }




