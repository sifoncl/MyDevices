// #include <WiFi.h>
// #include <ArduinoHA.h>

// #define BROKER_ADDR IPAddress(192, 168, 1, 100)
// #define PC_POW_PIN 35

// byte mac[] = {0x00, 0x10, 0xFA, 0x6E, 0x38, 0x4A};

// WiFiClient client;
// HADevice device(mac, sizeof(mac));
// HAMqtt mqtt(client, device);

// const char *WIFI_SSID = "DomWIFI";
// const char *WIFI_PASSWORD = "bigdadwsad1996";
// const char *MQTT_SERVER = "192.168.1.100";
// const char *MQTT_USER = "device";
// const char *MQTT_PASSWORD = "bigdadwsad1996";

// HABinarySensor sensor("myInput");

// bool state = false;
// bool lastInputState = false;
// unsigned long lastReadAt = millis();



// void setupWiFi()
// {
//     Serial.println();
//     Serial.print("Подключение к ");
//     Serial.println(WIFI_SSID);

//     WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

//     int attempts = 0;
//     while (WiFi.status() != WL_CONNECTED && attempts < 20)
//     {
//         delay(500);
//         Serial.print(".");
//         attempts++;
//     }

//     if (WiFi.status() == WL_CONNECTED)
//     {
//         Serial.println("");
//         Serial.println("Wi-Fi подключен");
//         Serial.print("IP-адрес: ");
//         Serial.println(WiFi.localIP());
//     }
//     else
//     {
//         Serial.println("");
//         Serial.println("Не удалось подключиться к Wi-Fi");
//         ESP.restart();
//     }
// }

// void setup()
// {
//     Serial.begin(115200);

//     // you don't need to verify return status
//     setupWiFi();

//     // set device's details (optional)
//     device.setName("Arduino");
//     device.setSoftwareVersion("1.0.0");

//     pinMode(PC_POW_PIN, INPUT_PULLUP);
//     lastInputState = digitalRead(PC_POW_PIN);

//     mqtt.begin(MQTT_SERVER, MQTT_USER, MQTT_PASSWORD);
// }

// void loop()
// {
//     Serial.println("Текущее состояния: ");
//     Serial.print(digitalRead(PC_POW_PIN));
//     Serial.println();

//         if ((millis() - lastReadAt) > 30) { // read in 30ms interval
//         // library produces MQTT message if a new state is different than the previous one
//         sensor.setState(digitalRead(PC_POW_PIN));
//         lastInputState = sensor.getCurrentState();
//         lastReadAt = millis();
//     }

//     mqtt.loop();
// }