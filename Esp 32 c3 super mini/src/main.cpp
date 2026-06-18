#include <Arduino.h>
#include <WiFi.h>

#define WIFI_SSID "DomWIFI"
#define WIFI_PASSWORD "bigdadwsad1996"
#define MQTT_SERVER "192.168.1.100"
#define MQTT_USER "device"
#define MQTT_PASSWORD "bigdadwsad1996"

#define RELAY_1_PIN 13
#define RELAY_2_PIN 12
#define RELAY_3_PIN 11
#define RELAY_4_PIN 10
#define SENSOR_PIN 9

void setupWiFi()
{
    Serial.println();
    Serial.print("Подключение к ");
    Serial.println(WIFI_SSID);
  delay(1000);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  delay(1000);
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
        Serial.println("Wi-Fi подключен");
        Serial.print("IP-адрес: ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        Serial.println("");
        Serial.println("Не удалось подключиться к Wi-Fi");
        ESP.restart();
    }
}

void setupPins(){
pinMode(RELAY_1_PIN, OUTPUT);
pinMode(RELAY_2_PIN, OUTPUT);
pinMode(RELAY_3_PIN, OUTPUT);
pinMode(RELAY_4_PIN, OUTPUT);
}




void setup()
{
    Serial.begin(115200);
    while(!Serial){
        delay(10);
    }
    Serial.println("Hello, ESP32111111!");  
  setupWiFi();        // Start serial communication at 115200 baud rate
  Serial.println("Hello, ESP322222222!"); // Print message to serial monitor
  pinMode(BUILTIN_LED, OUTPUT);        // Set LED_PIN as output
}

void loop()
{
 
  digitalWrite(RELAY_1_PIN, HIGH); //
  delay(200);
    digitalWrite(RELAY_2_PIN, HIGH); //
  delay(200);
    digitalWrite(RELAY_3_PIN, HIGH); //
  delay(200);
    digitalWrite(RELAY_4_PIN, HIGH); //
  delay(200);

  digitalWrite(RELAY_4_PIN, LOW); //
  delay(200);
    digitalWrite(RELAY_3_PIN, LOW); //
  delay(200);
    digitalWrite(RELAY_2_PIN, LOW); //
  delay(200);
    digitalWrite(RELAY_1_PIN, LOW); //
  delay(200);

  delay(1000); 
}
