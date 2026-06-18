#include <Arduino.h>
#include <Adafruit_MLX90393.h>
#include <SPI.h>
#include <WiFi.h>
#include <Wire.h>
#include <esp_now.h>
#include <esp_wifi.h>

#include "MotionTracker.h"
#include "RemoteSensorProtocol.h"
#include "SensorConfig.h"

#ifndef SENSOR_ROLE_ID
#define SENSOR_ROLE_ID 1
#endif

#ifndef MLX_TRANSPORT_SPI
#define MLX_TRANSPORT_SPI 0
#endif

Adafruit_MLX90393 mlx;
RemoteSensorPacket packet = {};
MotionTracker motionX;
MotionTracker motionY;
MotionTracker motionZ;

const uint8_t configuredControllerMac[6] = CONTROLLER_MAC;
const uint8_t broadcastMac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t targetMac[6] = {};

bool mlxAvailable = false;
bool espNowReady = false;
bool latestReadingOk = false;
volatile bool lastDeliverySucceeded = false;
volatile uint16_t consecutiveSendFailures = 0;
uint32_t sequenceNumber = 0;
unsigned long lastSampleAt = 0;
unsigned long lastTelemetryAt = 0;
unsigned long lastChannelScanAt = 0;
uint8_t currentChannel = FALLBACK_ESPNOW_CHANNEL;

bool macIsUnset(const uint8_t mac[6])
{
  for (uint8_t i = 0; i < 6; i++)
  {
    if (mac[i] != 0)
    {
      return false;
    }
  }
  return true;
}

void printMac(const uint8_t mac[6])
{
  Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

uint8_t discoverControllerChannel()
{
  const int networkCount = WiFi.scanNetworks(false, true);
  uint8_t discoveredChannel = FALLBACK_ESPNOW_CHANNEL;

  for (int i = 0; i < networkCount; i++)
  {
    if (WiFi.SSID(i) == CONTROLLER_LINK_AP_SSID)
    {
      discoveredChannel = static_cast<uint8_t>(WiFi.channel(i));
      break;
    }
  }

  WiFi.scanDelete();
  return discoveredChannel;
}

void setRadioChannel(uint8_t channel)
{
  if (channel < 1 || channel > 13)
  {
    channel = FALLBACK_ESPNOW_CHANNEL;
  }

  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);
  currentChannel = channel;
}

void onDataSent(const uint8_t *mac, esp_now_send_status_t status)
{
  (void)mac;
  lastDeliverySucceeded = status == ESP_NOW_SEND_SUCCESS;
  if (lastDeliverySucceeded)
  {
    consecutiveSendFailures = 0;
  }
  else if (consecutiveSendFailures < UINT16_MAX)
  {
    consecutiveSendFailures++;
  }
}

bool initializeEspNow()
{
  if (esp_now_init() != ESP_OK)
  {
    return false;
  }

  esp_now_register_send_cb(onDataSent);

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, targetMac, 6);
  peer.channel = 0;
  peer.ifidx = WIFI_IF_STA;
  peer.encrypt = false;

  return esp_now_add_peer(&peer) == ESP_OK ||
         esp_now_is_peer_exist(targetMac);
}

bool initializeMlx()
{
#if MLX_TRANSPORT_SPI
  SPI.begin(MLX_SPI_SCK_PIN, MLX_SPI_MISO_PIN, MLX_SPI_MOSI_PIN, MLX_SPI_CS_PIN);
  const bool started = mlx.begin_SPI(MLX_SPI_CS_PIN, &SPI);
#else
  Wire.begin(MLX_I2C_SDA_PIN, MLX_I2C_SCL_PIN);
  const bool started = mlx.begin_I2C(MLX_I2C_ADDRESS, &Wire);
#endif

  if (!started)
  {
    return false;
  }

  mlx.setGain(MLX90393_GAIN_1X);
  mlx.setResolution(MLX90393_X, MLX90393_RES_17);
  mlx.setResolution(MLX90393_Y, MLX90393_RES_17);
  mlx.setResolution(MLX90393_Z, MLX90393_RES_16);
  mlx.setOversampling(MLX90393_OSR_2);
  mlx.setFilter(MLX90393_FILTER_3);
  return true;
}

void refreshControllerChannel()
{
  const uint8_t discoveredChannel = discoverControllerChannel();
  setRadioChannel(discoveredChannel);
  lastChannelScanAt = millis();

  Serial.print("ESP-NOW channel: ");
  Serial.println(currentChannel);
}

void readSensor()
{
  sensors_event_t event;
  latestReadingOk = mlxAvailable && mlx.getEvent(&event);
  if (!latestReadingOk)
  {
    return;
  }

  const unsigned long nowMs = millis();
  motionX.update(event.magnetic.x, nowMs);
  motionY.update(event.magnetic.y, nowMs);
  motionZ.update(event.magnetic.z, nowMs);
}

void sendTelemetry()
{
  packet.magic = REMOTE_SENSOR_MAGIC;
  packet.version = REMOTE_SENSOR_PROTOCOL_VERSION;
  packet.role = SENSOR_ROLE_ID;
  packet.transport = MLX_TRANSPORT_SPI
                         ? static_cast<uint8_t>(RemoteSensorTransport::SPI)
                         : static_cast<uint8_t>(RemoteSensorTransport::I2C);
  packet.flags = latestReadingOk ? REMOTE_SENSOR_FLAG_SENSOR_OK : 0;
  packet.sequence = ++sequenceNumber;
  packet.uptimeMs = millis();
  packet.x = motionX.filtered();
  packet.y = motionY.filtered();
  packet.z = motionZ.filtered();
  packet.velocityX = motionX.velocity();
  packet.velocityY = motionY.velocity();
  packet.velocityZ = motionZ.velocity();
  packet.directionX = motionX.direction();
  packet.directionY = motionY.direction();
  packet.directionZ = motionZ.direction();
  packet.movingAxes = 0;
  if (motionX.moving())
  {
    packet.movingAxes |= REMOTE_SENSOR_MOVING_X;
  }
  if (motionY.moving())
  {
    packet.movingAxes |= REMOTE_SENSOR_MOVING_Y;
  }
  if (motionZ.moving())
  {
    packet.movingAxes |= REMOTE_SENSOR_MOVING_Z;
  }

  if (espNowReady)
  {
    const esp_err_t result =
        esp_now_send(targetMac,
                     reinterpret_cast<const uint8_t *>(&packet),
                     sizeof(packet));
    if (result != ESP_OK && consecutiveSendFailures < UINT16_MAX)
    {
      consecutiveSendFailures++;
    }
  }
}

void setup()
{
  Serial.begin(115200);
  delay(600);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.println(SENSOR_ROLE_ID == 1 ? "Upper gate sensor" : "Lower gate sensor");
  Serial.print("Sensor STA MAC: ");
  Serial.println(WiFi.macAddress());

  if (macIsUnset(configuredControllerMac))
  {
    memcpy(targetMac, broadcastMac, 6);
    Serial.println("Controller MAC is unset, using broadcast");
  }
  else
  {
    memcpy(targetMac, configuredControllerMac, 6);
    Serial.print("Controller MAC: ");
    printMac(targetMac);
    Serial.println();
  }

  refreshControllerChannel();
  espNowReady = initializeEspNow();
  mlxAvailable = initializeMlx();

  Serial.println(espNowReady ? "ESP-NOW ready" : "ESP-NOW initialization failed");
  Serial.println(mlxAvailable ? "MLX90393 ready" : "MLX90393 not found");
  Serial.println(MLX_TRANSPORT_SPI ? "Transport: SPI" : "Transport: I2C");
}

void loop()
{
  const unsigned long nowMs = millis();

  if (consecutiveSendFailures >= CHANNEL_RESCAN_FAILURE_COUNT &&
      (nowMs - lastChannelScanAt) >= CHANNEL_RESCAN_COOLDOWN_MS)
  {
    refreshControllerChannel();
    consecutiveSendFailures = 0;
  }

  if ((nowMs - lastSampleAt) >= SENSOR_SAMPLE_INTERVAL_MS)
  {
    lastSampleAt = nowMs;
    readSensor();
  }

  if ((nowMs - lastTelemetryAt) >= TELEMETRY_INTERVAL_MS)
  {
    lastTelemetryAt = nowMs;
    sendTelemetry();
  }

  static unsigned long lastDebugAt = 0;
  if ((nowMs - lastDebugAt) >= 2000UL)
  {
    lastDebugAt = nowMs;
    Serial.printf("seq=%lu sensor=%s delivery=%s channel=%u dir=%d/%d/%d moving=0x%02X\r\n",
                  static_cast<unsigned long>(sequenceNumber),
                  latestReadingOk ? "ok" : "error",
                  lastDeliverySucceeded ? "ok" : "unknown/fail",
                  currentChannel,
                  motionX.direction(),
                  motionY.direction(),
                  motionZ.direction(),
                  packet.movingAxes);
  }
}
