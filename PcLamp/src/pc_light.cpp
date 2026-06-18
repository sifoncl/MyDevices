#include <WiFi.h>
#include <ArduinoHA.h>
#include <RGBWWWController.h>
#include <Wire.h>
#include <dht.h>

#if __has_include("Secrets.h")
#include "Secrets.h"
#endif

#ifndef WIFI_SSID
#define WIFI_SSID "CHANGE_ME"
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "CHANGE_ME"
#endif
#ifndef MQTT_SERVER
#define MQTT_SERVER "192.168.1.100"
#endif
#ifndef MQTT_USER
#define MQTT_USER "CHANGE_ME"
#endif
#ifndef MQTT_PASSWORD
#define MQTT_PASSWORD "CHANGE_ME"
#endif

#define DEVICE_NAME "pc lamp and power conroller"
#define DEVICE_ID "custom_1"
#define SOFTVARE_VERSION "1.0.0"

#define PC_POW_PIN 12
#define SHOUTDOUN_TIME 10
#define BUTTON_PRESSING_TIME 200
#define ATTEMPS_PRESSES_TO_START 1
#define PC_POW_BUTON_PIN 14

#define RED_PIN 18
#define GREEN_PIN 19
#define BLUE_PIN 5
#define WHITE_PIN 17
#define WARM_WHITE_PIN 21
#define MAX_COLOR_TEMP 454
#define MIN_COLOR_TEMP 153

#define DHT_DATA_PIN 25
#define SENSOR_SHUTDOWN_TIMER 10

#define TOUCH_BUTTON_PIN 32
#define TOUCH_THRESHOLD 10
#define DEBOUNCE_TIME 100

byte mac[] = {0x00, 0x10, 0xFA, 0x6E, 0x38, 0x4A};

WiFiClient client;
HADevice device(mac, sizeof(mac));
HAMqtt mqtt(client, device);

HASensorNumber tempSensor("badroomTemp", HASensorNumber::PrecisionP1);
HASensorNumber humdSensor("badroomHumd", HASensorNumber::PrecisionP1);
HALight light("PcLamp", HALight::BrightnessFeature | HALight::ColorTemperatureFeature | HALight::RGBFeature);
HASwitch pCswitch("MyPC");

RGBWWWController ledStrip(RED_PIN, GREEN_PIN, BLUE_PIN, WHITE_PIN, WARM_WHITE_PIN);

DHT sensor(DHT_DATA_PIN, DHT22);

float temp = 0;
float humid = 0;
unsigned long lastUpdateAt = 0;
unsigned long unavailebleTimer = 0;
boolean isStartUnavaileble = false;

unsigned long lastTouchTime = 0;
bool touchState = false;
bool lastTouchState = false;
int touchValue = 0;

boolean isPcOn = false;
boolean isShotingDoun = false;
unsigned long shoutDounBeginAt = 0;
uint8_t attempsToSutDown = 0;

boolean isButtonPressing = false;
unsigned long pressButtonStarted = 0;

boolean isPcTurningOn = false;
uint8_t attempsToStart = 0;
unsigned long startBeginAt = 0;

String

void pinSetup()
{
    pinMode(PC_POW_PIN, INPUT_PULLUP);
}

void pressButton()
{
    pressButtonStarted = millis();
    if (!isButtonPressing)
    {
        isButtonPressing = true;
    }
}

void turnOnPc()
{
    Serial.println("Включаю ПК");
    isPcTurningOn = true;
    startBeginAt = millis();
}

void turnOffPc()
{
    Serial.println("Вы_ключаю ПК");
    isShotingDoun = true;
    shoutDounBeginAt = millis();
    pressButton();
}

void onSwitchCommand(bool state, HASwitch *sender)
{
    // if (sender == &pCswitch)

    if (state && !isPcOn)
    {
        turnOnPc();
    }
    if (!state && isPcOn && !isShotingDoun)
    {
        turnOffPc();
    }

 //   sender->setState(state); // report state back to the Home Assistant
}

void onStateCommand(bool state, HALight *sender)
{
    Serial.println();
    Serial.println("State: ");
    Serial.println(state);
    Serial.println();

    if (state)
    {
        ledStrip.turnOn();
    }
    else
    {
        ledStrip.turnOff();
    }
    sender->setState(ledStrip.isOn()); // report state back to the Home Assistant
}

void onBrightnessCommand(uint8_t brightness, HALight *sender)
{
    Serial.print("Brightness: ");
    Serial.println(brightness);

    ledStrip.setBrightness(brightness);
    sender->setBrightness(ledStrip.getBrightness()); // report brightness back to the Home Assistant
}

void onColorTemperatureCommand(uint16_t temperature, HALight *sender)
{
    Serial.println("Color temperature: ");
    Serial.print(temperature);

    ledStrip.setColorTempAndChangeMode(temperature);
    sender->setColorTemperature(temperature); // report color temperature back to the Home Assistant
}

void onRGBColorCommand(HALight::RGBColor color, HALight *sender)
{
    Serial.print("Red: ");
    Serial.println(color.red);
    Serial.print("Green: ");
    Serial.println(color.green);
    Serial.print("Blue: ");
    Serial.println(color.blue);

    ledStrip.setRGBandChageMode(color.red, color.green, color.blue);
    sender->setRGBColor(color); // report color back to the Home Assistant
}

void ledStripLoop()
{
    light.setState(ledStrip.isOn());
}

void pCcheckLoop()
{
    isPcOn = digitalRead(PC_POW_PIN);
    // Serial.println("isPcOn ");
    // Serial.print(isPcOn);
    // Serial.println(digitalRead(PC_POW_PIN));
    // Serial.println();

    boolean stateTosend = isPcOn;

    if (isShotingDoun)
    {
        if (!isPcOn)
        {
            isShotingDoun = false;
            attempsToSutDown = 0;
        }
        else if (((millis() - shoutDounBeginAt) > SHOUTDOUN_TIME * 1000) && attempsToSutDown <= 1)
        {
            Serial.print("Попытка выключить пк ");
            Serial.println(attempsToSutDown);
            pressButton();
            attempsToSutDown++;
            shoutDounBeginAt = millis();
        }
        else if (attempsToSutDown > ATTEMPS_PRESSES_TO_START)
        {
            Serial.println("Неудалось выключить пк ");
            attempsToSutDown = 0;
            isShotingDoun = false;
        }
        stateTosend = false;
    }

    if (isPcTurningOn)
    {
        if (isPcOn)
        {
            isPcTurningOn = false;
            attempsToStart = 0;
            startBeginAt = 0;
        }
        else if (((millis() - shoutDounBeginAt) > SHOUTDOUN_TIME * 1000) && attempsToStart <= 1 && !isPcOn)
        {
            Serial.print("Попытка в_ключить ПК ");
            Serial.println(attempsToStart);
            pressButton();
            attempsToStart++;
            stateTosend = true;
        }
    }

     pCswitch.setState(stateTosend);
    }
void touchButtonLoop()
{
    touchValue = touchRead(TOUCH_BUTTON_PIN);
    //Serial.println(touchValue);
    bool currentTouch = (touchValue < TOUCH_THRESHOLD);

    // Антидребезг
    if (currentTouch != lastTouchState)
    {
        lastTouchTime = millis();
    }

    if ((millis() - lastTouchTime) > DEBOUNCE_TIME)
    {
        if (currentTouch != touchState)
        {
            touchState = currentTouch;

            if (touchState)
            {
                Serial.println("Touch button pressed");

                // Переключение света
                if (light.getCurrentState())
                {
                    ledStrip.turnOff();
                    light.setState(ledStrip.isOn(), true);
                    Serial.println("Light turned OFF");
                }
                else
                {
                    ledStrip.turnOn();
                    light.setState(ledStrip.isOn(), true);
                    Serial.println("Light turned ON");
                }
            }
        }
    }

    lastTouchState = currentTouch;
}

void pcPressButtonLoop()
{
    // Serial.println(isButtonPressing);
    if (((millis() - pressButtonStarted) < BUTTON_PRESSING_TIME) && isButtonPressing)
    {
        // Serial.println("Кнопка нажата");
        pinMode(PC_POW_BUTON_PIN, OUTPUT);
        digitalWrite(PC_POW_BUTON_PIN, true);
    }
    else
    {
        // Serial.println("Кнопка отжата");
        isButtonPressing = false;
        digitalWrite(PC_POW_BUTON_PIN, false);
        pinMode(PC_POW_BUTON_PIN, INPUT_PULLDOWN);
    }
}

void sensorLoop()
{
    if ((millis() - lastUpdateAt) > 4200)
    {
        lastUpdateAt = millis();

        temp = sensor.readTemperature();
        humid = sensor.readHumidity();

        if (isnan(temp) || isnan(humid))
        {
            temp = 1000;
            humid = 1000;
        }
        if (temp < 100 && humid < 105)
        {
            Serial.print(F("Temperature: "));
            Serial.print(temp);
            Serial.println("°C");

            Serial.print(F("Humidity: "));
            Serial.print(humid);
            Serial.println("%");

            tempSensor.setAvailability(true);
            humdSensor.setAvailability(true);
            tempSensor.setValue(temp);
            humdSensor.setValue(humid);
            isStartUnavaileble = false;
        }
        else if ((temp > 100 || humid > 105) && !isStartUnavaileble)
        {
            Serial.println("Датчик выдал не правильные данные");
            isStartUnavaileble = true;
            unavailebleTimer = millis();
        }

        if (isStartUnavaileble && ((millis() - unavailebleTimer) > SENSOR_SHUTDOWN_TIMER * 1000))
        {
            if (tempSensor.isOnline() || humdSensor.isOnline())
            {
                Serial.println("Датчик отключен");
                tempSensor.setAvailability(false);
                humdSensor.setAvailability(false);
                unavailebleTimer = 0;
            }
        }
    }
}

void lightConfig()
{

    light.setName("Свет над пк");
    light.setIcon("mdi:desk-lamp-on");

    // Optionally you can set retain flag for the HA commands
    // light.setRetain(true);

    // Maximum brightness level can be changed as follows:
    light.setBrightnessScale(255);

    // Color temperature range (optional)
    light.setMinMireds(MIN_COLOR_TEMP);
    light.setMaxMireds(MAX_COLOR_TEMP);

    light.onStateCommand(onStateCommand);
    light.onBrightnessCommand(onBrightnessCommand);
    light.onColorTemperatureCommand(onColorTemperatureCommand);
    light.onRGBColorCommand(onRGBColorCommand);

    ledStrip.setTempParams(MAX_COLOR_TEMP, MIN_COLOR_TEMP, true);
    ledStrip.setWWCWmode();
    ledStrip.setWhite(255, 255, 64);
}
void sensorConfig()
{
    sensor.begin();
    tempSensor.setIcon("mdi:thermometer");
    tempSensor.setName("Температура в спальне");
    tempSensor.setUnitOfMeasurement("°C");
    tempSensor.setDeviceClass("temperature");

    humdSensor.setIcon("mdi:water-percent");
    humdSensor.setName("Влжность в спальне");
    humdSensor.setUnitOfMeasurement("%");
    humdSensor.setDeviceClass("humidity");
}

void pcSwitchConfig()
{
    pCswitch.setName("Kolya PC");
    pCswitch.setIcon("mdi:desktop-classic");
    pCswitch.onCommand(onSwitchCommand);
}

void deviceConfig()
{
    device.setName(DEVICE_NAME);
    device.setSoftwareVersion(SOFTVARE_VERSION);
}

void setupWiFi()
{
    Serial.println();
    Serial.print("Подключение к ");
    Serial.println(WIFI_SSID);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

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

void setup()
{
    Serial.begin(115200);

    setupWiFi();
    Serial.println("Прошла настройка Wifi");
    deviceConfig();
    Serial.println("Прошла настройка устройства");
    lightConfig();
    Serial.println("Прошла настройка ленты");
    sensorConfig();
    Serial.println("Прошла настройка датчика");
    pcSwitchConfig();
    Serial.println("Прошла настройка кнопки пк");
    mqtt.begin(MQTT_SERVER, MQTT_USER, MQTT_PASSWORD);
}

void loop()
{
    sensorLoop();
    touchButtonLoop();
    pCcheckLoop();
    pcPressButtonLoop();
    ledStripLoop();
    mqtt.loop();
}
