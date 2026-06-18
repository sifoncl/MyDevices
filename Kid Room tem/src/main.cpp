#include <Arduino.h>
#include <WiFi.h>
#include "WiFiUtils.h"
#include "SensorWrapper.h"
#include "DisplayScreens.h"
#include <ArduinoHA.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <SensirionI2cScd4x.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

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

#define DEVICE_NAME "Анализатор микроклимата в столовой"
#define DEVICE_ID "custom_9"
#define SOFTVARE_VERSION "1.0.0"

#define SDA_PIN 12
#define SCL_PIN 13
#define SCREEN_TOUCH_PIN 7
#define SCREEN_TOUCH_DEBOUNCE_MS 50UL
#define SCREEN_TOUCH_THRESHOLD_DELTA 5000
#define SCREEN_TOUCH_DEBUG_INTERVAL_MS 500UL
#define WIFI_CONNECT_ATTEMPTS 20
#define WIFI_CONNECT_RETRY_DELAY_MS 500UL
#define WIFI_RECONNECT_INTERVAL_MS 10000UL

#define ENV_SENSOR_ADDRESS 0x76
#define SENSOR_UPDATE_INTERVAL 4200UL
#define CO2_UPDATE_INTERVAL 5000UL
#define BOOT_INFO_SCREEN_MS 5000UL
#define BOOT_HA_STATE_WAIT_AFTER_CONNECT_MS 1500UL
#define BOOT_MAX_WAIT_MS 12000UL
#define SENSOR_SHUTDOWN_TIMER 10
#define SENSOR_UNAVAILABLE_DELAY_MS (SENSOR_SHUTDOWN_TIMER * 1000UL)
#define SCD4X_OK 0
#define CO2_ALARM_THRESHOLD_PPM 1000
#define TIME_UTC_OFFSET_HOURS 3
#define TIME_AUTO_DST false

byte mac[] = {0x09, 0x10, 0xFA, 0x6E, 0x38, 0x4A};

WiFiClient client;
HADevice device(mac, sizeof(mac));
HAMqtt mqtt(client, device);

Adafruit_BME280 environmentSensor;
SensirionI2cScd4x scd41Sensor;
WiFiUDP timeUdp;
InternetClock internetClock(timeUdp, TIME_UTC_OFFSET_HOURS, TIME_AUTO_DST);

float readTemperature(Adafruit_BME280 &sensor)
{
  return sensor.readTemperature();
}

float readHumidity(Adafruit_BME280 &sensor)
{
  return sensor.readHumidity();
}

float readPressureMmHg(Adafruit_BME280 &sensor)
{
  return sensor.readPressure() / 133.322368F;
}

float readCo2(SensirionI2cScd4x &sensor)
{
  bool dataReady = false;
  uint16_t co2 = 0;
  float temperature = 0;
  float humidity = 0;

  if (sensor.getDataReadyStatus(dataReady) != SCD4X_OK || !dataReady)
  {
    return NAN;
  }

  if (sensor.readMeasurement(co2, temperature, humidity) != SCD4X_OK || co2 == 0)
  {
    return NAN;
  }

  return static_cast<float>(co2);
}

SensorWrapper<Adafruit_BME280> temperatureValue(environmentSensor, readTemperature, SENSOR_UPDATE_INTERVAL, SENSOR_UNAVAILABLE_DELAY_MS);
SensorWrapper<Adafruit_BME280> humidityValue(environmentSensor, readHumidity, SENSOR_UPDATE_INTERVAL, SENSOR_UNAVAILABLE_DELAY_MS);
SensorWrapper<Adafruit_BME280> pressureValue(environmentSensor, readPressureMmHg, SENSOR_UPDATE_INTERVAL, SENSOR_UNAVAILABLE_DELAY_MS);
SensorWrapper<SensirionI2cScd4x> co2Value(scd41Sensor, readCo2, CO2_UPDATE_INTERVAL, SENSOR_UNAVAILABLE_DELAY_MS);

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

StartupIpScreen startupIpScreen;
BigClockScreen bigClockScreen(internetClock);
DateClockScreen dateClockScreen(internetClock);
SensorValueScreen<Adafruit_BME280> temperatureScreen(temperatureValue, "Temp", "C", 1);
SensorValueScreen<Adafruit_BME280> humidityScreen(humidityValue, "Humidity", "%", 1);
SensorValueScreen<Adafruit_BME280> pressureScreen(pressureValue, "Pressure", "mmHg", 1, 1);
SensorValueScreen<SensirionI2cScd4x> co2Screen(co2Value, "CO2", "ppm", 0);

DisplayScreen *mainScreens[] = {
    &bigClockScreen,
    &dateClockScreen,
    &temperatureScreen,
    &humidityScreen,
    &pressureScreen,
    &co2Screen};
const uint8_t MAIN_SCREEN_COUNT = sizeof(mainScreens) / sizeof(mainScreens[0]);
const uint8_t CO2_SCREEN_INDEX = MAIN_SCREEN_COUNT - 1;

TouchButton screenTouchButton(SCREEN_TOUCH_PIN, SCREEN_TOUCH_THRESHOLD_DELTA, SCREEN_TOUCH_DEBOUNCE_MS);
ScreenController screenController(display, screenTouchButton, startupIpScreen, mainScreens, MAIN_SCREEN_COUNT);

// devices types go here
HASwitch switch1("DinerRoomScreen");

HASensorNumber tempSensor("DinerRoomTemp", HASensorNumber::PrecisionP1);
HASensorNumber humdSensor("DinerRoomHumd", HASensorNumber::PrecisionP1);
HASensorNumber presSensor("DinerRoomPressure", HASensorNumber::PrecisionP1);
HASensorNumber co2HaSensor("DinerRoomCO2", HASensorNumber::PrecisionP0);

bool bootSequenceCompleted = false;
bool bootScreenStateReceived = false;
bool bootRestoredScreenEnabled = true;
bool bootMqttConnected = false;
unsigned long bootInfoScreenStartedAt = 0;
unsigned long bootMqttConnectedAt = 0;

void onScreenEnabledChanged(bool enabled)
{
  switch1.setState(enabled);
}

void onMqttConnected()
{
  bootMqttConnected = true;
  bootMqttConnectedAt = millis();

  if (bootSequenceCompleted)
  {
    switch1.setState(screenController.isEnabled(), true);
  }
}

void deviceConfig()
{
  device.setName(DEVICE_NAME);
  device.setSoftwareVersion(SOFTVARE_VERSION);
}

void onSwitchCommand(bool state, HASwitch *sender)
{
  if (sender == &switch1)
  {
    if (!bootSequenceCompleted)
    {
      switch1.setCurrentState(state);
      bootRestoredScreenEnabled = state;
      bootScreenStateReceived = true;
      return;
    }

    screenController.setEnabled(state);
  }
}

void drawCenteredText(const String &text, uint8_t textSize, int16_t y)
{
  int16_t x1 = 0;
  int16_t y1 = 0;
  uint16_t width = 0;
  uint16_t height = 0;

  display.setTextSize(textSize);
  display.getTextBounds(text, 0, y, &x1, &y1, &width, &height);
  display.setCursor((display.width() - static_cast<int16_t>(width)) / 2 - x1, y);
  display.println(text);
}

void drawDisplayScreen(DisplayScreen &screen)
{
  display.ssd1306_command(SSD1306_DISPLAYON);
  display.clearDisplay();
  screen.draw(display);
  display.display();
}

void showLoadingScreen()
{
  display.ssd1306_command(SSD1306_DISPLAYON);
  display.clearDisplay();
  display.setTextColor(WHITE);
  drawCenteredText("Loading...", 2, 24);
  display.display();
}

void showBootInfoScreen()
{
  startupIpScreen.setIp(WiFi.localIP());
  drawDisplayScreen(startupIpScreen);
  bootInfoScreenStartedAt = millis();
}

void finishBootSequence()
{
  const bool screenEnabled = bootScreenStateReceived ? bootRestoredScreenEnabled : true;

  bootSequenceCompleted = true;
  screenController.begin(WiFi.localIP(), false);
  screenController.setEnabled(screenEnabled);

  if (mqtt.isConnected())
  {
    switch1.setState(screenEnabled, true);
  }
}

void bootLoop()
{
  if (bootSequenceCompleted)
  {
    return;
  }

  const unsigned long nowMs = millis();
  const bool infoScreenFinished = (nowMs - bootInfoScreenStartedAt) >= BOOT_INFO_SCREEN_MS;
  const bool wifiUnavailable = !WiFiUtils::isConnected();
  const bool waitedForHaAfterConnect = bootMqttConnected &&
                                       (nowMs - bootMqttConnectedAt) >= BOOT_HA_STATE_WAIT_AFTER_CONNECT_MS;
  const bool bootTimedOut = (nowMs - bootInfoScreenStartedAt) >= BOOT_MAX_WAIT_MS;

  if (!infoScreenFinished)
  {
    return;
  }

  if (wifiUnavailable || bootScreenStateReceived || waitedForHaAfterConnect || bootTimedOut)
  {
    finishBootSequence();
  }
}

template <typename TSensor>
void updateHaSensor(SensorWrapper<TSensor> &wrappedSensor, HASensorNumber &haSensor, const char *name, const char *unit)
{
  if (!wrappedSensor.loop())
  {
    return;
  }

  Serial.println(wrappedSensor.lastRawValue());

  if (wrappedSensor.isAvailable())
  {
    const float value = wrappedSensor.value();
    Serial.print(name);
    Serial.print(": ");
    Serial.print(value);
    Serial.println(unit);

    haSensor.setAvailability(true);
    haSensor.setValue(value);
    return;
  }

  Serial.print(name);
  Serial.println(": датчик выдал неправильные данные");

  if (haSensor.isOnline())
  {
    Serial.print(name);
    Serial.println(": датчик отключен");
    haSensor.setAvailability(false);
  }
}

void sensorLoop()
{
  updateHaSensor(temperatureValue, tempSensor, "Temperature", " °C");
  updateHaSensor(humidityValue, humdSensor, "Humidity", "%");
  updateHaSensor(pressureValue, presSensor, "Pressure", " mmHg");
  updateHaSensor(co2Value, co2HaSensor, "CO2", " ppm");
}

void co2AlarmLoop()
{
  const bool alarmActive = co2Value.isAvailable() &&
                           co2Value.hasValue() &&
                           co2Value.value() >= CO2_ALARM_THRESHOLD_PPM;

  screenController.setAlarmMode(alarmActive, CO2_SCREEN_INDEX);
}

void setupSwitch()
{
  switch1.setName("Screen");
  switch1.setIcon("mdi:lightbulb");
  switch1.setRetain(true);
  switch1.onCommand(onSwitchCommand);
  mqtt.onConnected(onMqttConnected);
  screenController.setOnEnabledChanged(onScreenEnabledChanged);
}

void sensorConfig()
{
  if (!environmentSensor.begin(ENV_SENSOR_ADDRESS))
  {
    Serial.println("BME280 не найден! Проверьте подключение.");
  }

  scd41Sensor.begin(Wire, SCD41_I2C_ADDR_62);
  delay(30);
  scd41Sensor.wakeUp();
  scd41Sensor.stopPeriodicMeasurement();
  scd41Sensor.reinit();

  uint64_t scd41SerialNumber = 0;
  const int16_t scd41Error = scd41Sensor.getSerialNumber(scd41SerialNumber);
  if (scd41Error != SCD4X_OK)
  {
    Serial.println("SCD41 не найден! Проверьте подключение.");
  }
  else if (scd41Sensor.startPeriodicMeasurement() != SCD4X_OK)
  {
    Serial.println("SCD41 не запустил периодические измерения.");
  }

  tempSensor.setIcon("mdi:thermometer");
  tempSensor.setName("Температура");
  tempSensor.setUnitOfMeasurement("°C");
  tempSensor.setDeviceClass("temperature");

  humdSensor.setIcon("mdi:water-percent");
  humdSensor.setName("Влажность");
  humdSensor.setUnitOfMeasurement("%");
  humdSensor.setDeviceClass("humidity");

  presSensor.setIcon("mdi:speedometer-medium");
  presSensor.setName("Атмосферное давление");
  presSensor.setUnitOfMeasurement("mmHg");
  presSensor.setDeviceClass("pressure");

  co2HaSensor.setIcon("mdi:molecule-co2");
  co2HaSensor.setName("CO2");
  co2HaSensor.setUnitOfMeasurement("ppm");
  co2HaSensor.setDeviceClass("carbon_dioxide");

  temperatureValue.setValidRange(-40, 85);
  humidityValue.setValidRange(0, 100);
  pressureValue.setValidRange(300, 900);
  co2Value.setValidRange(1, 40000);
}

void screenConfig()
{
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  delay(2000);
  display.clearDisplay();
  display.display();
  display.ssd1306_command(SSD1306_DISPLAYOFF);
}

void setupScreenController()
{
  startupIpScreen.setDeviceName(DEVICE_NAME);
  startupIpScreen.setDeviceInfo(DEVICE_ID, SOFTVARE_VERSION);
  screenTouchButton.begin();
}

void touchDebugLoop()
{
  if (!screenController.isEnabled())
  {
    return;
  }

  static unsigned long lastPrintAt = 0;

  if ((millis() - lastPrintAt) < SCREEN_TOUCH_DEBUG_INTERVAL_MS)
  {
    return;
  }
  lastPrintAt = millis();

  Serial.print("Touch raw: ");
  Serial.print(screenTouchButton.lastTouchValue());
  Serial.print(" | filtered: ");
  Serial.print(screenTouchButton.filteredValue());
  Serial.print(" | baseline: ");
  Serial.print(screenTouchButton.baseline());
  Serial.print(" | press threshold: ");
  Serial.print(screenTouchButton.threshold());
  Serial.print(" | release threshold: ");
  Serial.print(screenTouchButton.releaseThreshold());
  Serial.print(" | delta: ");
  Serial.print(SCREEN_TOUCH_THRESHOLD_DELTA);
  Serial.print(" | pressed: ");
  Serial.println(screenTouchButton.isPressed() ? "yes" : "no");
}

void setup()
{
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  screenConfig();
  showLoadingScreen();
  setupScreenController();
  setupSwitch();
  deviceConfig();
  sensorConfig();

  WiFiUtils::beginAutoReconnect(WIFI_SSID, WIFI_PASSWORD, WIFI_RECONNECT_INTERVAL_MS);
  WiFiUtils::connect(WIFI_SSID, WIFI_PASSWORD, WIFI_CONNECT_ATTEMPTS, WIFI_CONNECT_RETRY_DELAY_MS, false);
  if (WiFiUtils::isConnected())
  {
    internetClock.begin();
  }
  mqtt.begin(MQTT_SERVER, MQTT_USER, MQTT_PASSWORD);

  showBootInfoScreen();
}

void loop()
{
  WiFiUtils::loop();
  mqtt.loop();
  bootLoop();
  sensorLoop();
  internetClock.loop();
  if (bootSequenceCompleted)
  {
    co2AlarmLoop();
    screenController.loop();
  }
  touchDebugLoop();
}
