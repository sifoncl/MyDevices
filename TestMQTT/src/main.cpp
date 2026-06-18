#include <WiFi.h>
#include <PubSubClient.h>

// Настройки Wi-Fi
const char* ssid = "ВАШ_WIFI_SSID";
const char* password = "ВАШ_WIFI_ПАРОЛЬ";

// Настройки MQTT
const char* mqtt_server = "mqtt.server.com"; // Адрес MQTT-брокера
const int mqtt_port = 1883; // Порт MQTT-брокера
const char* mqtt_user = "username"; // Имя пользователя MQTT (если требуется)
const char* mqtt_pass = "password"; // Пароль MQTT (если требуется)

// Топики для подписки и публикации
const char* topic_sub = "esp32/sub"; // Топик для получения сообщений
const char* topic_pub = "esp32/pub"; // Топик для отправки сообщений

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Подключение к ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Wi-Fi подключен");
  Serial.print("IP-адрес: ");
  Serial.println(WiFi.localIP());
}

// Функция обработки входящих MQTT-сообщений
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Сообщение получено [");
  Serial.print(topic);
  Serial.print("]: ");
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Попытка подключения к MQTT...");
    
    // Генерируем случайный ID клиента
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("подключено!");
      
      // Подписываемся на топик
      client.subscribe(topic_sub);
    } else {
      Serial.print("ошибка, код: ");
      Serial.print(client.state());
      Serial.println(" пробуем снова через 5 секунд");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Отправляем сообщение каждые 5 секунд
  static unsigned long lastMsg = 0;
  unsigned long now = millis();
  
  if (now - lastMsg > 5000) {
    lastMsg = now;
    
    String msg = "Время работы: " + String(now/1000) + " сек";
    client.publish(topic_pub, msg.c_str());
    Serial.println("Сообщение отправлено: " + msg);
  }
}