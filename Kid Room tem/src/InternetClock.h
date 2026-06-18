#ifndef INTERNET_CLOCK_H
#define INTERNET_CLOCK_H

#include <Arduino.h>
#include <WiFiUdp.h>
#include <time.h>

struct InternetClockDateTime
{
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  uint8_t weekDay; // 0 - Sunday, 6 - Saturday.
  bool isDst;
  int16_t utcOffsetMinutes;
};

class InternetClock
{
public:
  enum class OffsetUnit : uint8_t
  {
    Hours,
    Minutes
  };

  // utcOffset is the winter/base offset. If autoDst is enabled, summer time adds 60 minutes.
  InternetClock(WiFiUDP &udp, int16_t utcOffset, OffsetUnit unit = OffsetUnit::Hours, bool autoDst = true, const char *ntpServer = "pool.ntp.org");
  InternetClock(WiFiUDP &udp, int8_t utcOffsetHours, bool autoDst, const char *ntpServer = "pool.ntp.org");

  bool begin(uint16_t localPort = 2390);
  void loop();
  bool syncNow();

  bool isSynced() const;
  bool isDst() const;

  void setNtpServer(const char *ntpServer);
  void setSyncInterval(unsigned long intervalMs);
  void setAutoDst(bool enabled);
  void setBaseUtcOffset(int16_t utcOffset, OffsetUnit unit = OffsetUnit::Hours);

  uint32_t utcEpoch() const;
  uint32_t localEpoch() const;
  InternetClockDateTime now() const;

  uint16_t year() const;
  uint8_t month() const;
  uint8_t day() const;
  uint8_t hour() const;
  uint8_t minute() const;
  uint8_t second() const;
  uint8_t weekDay() const;

  String timeString() const;     // HH:MM:SS
  String shortTimeString() const; // HH:MM
  String dateString() const;     // YYYY-MM-DD
  String dateTimeString() const; // YYYY-MM-DD HH:MM:SS
  String isoString() const;      // YYYY-MM-DDTHH:MM:SS+03:00

  String format(const char *pattern) const;
  bool formatTo(char *buffer, size_t length, const char *pattern) const;

private:
  static const uint16_t NTP_PORT = 123;
  static const size_t NTP_PACKET_SIZE = 48;
  static const uint32_t NTP_UNIX_OFFSET = 2208988800UL;
  static const unsigned long DEFAULT_SYNC_INTERVAL_MS = 10UL * 60UL * 1000UL;
  static const unsigned long RETRY_INTERVAL_MS = 15UL * 1000UL;
  static const unsigned long NTP_TIMEOUT_MS = 1500UL;

  WiFiUDP &_udp;
  const char *_ntpServer;
  int16_t _baseUtcOffsetMinutes;
  bool _autoDst;
  bool _udpStarted;
  bool _synced;
  uint16_t _localPort;
  uint32_t _utcEpochAtSync;
  unsigned long _syncedAtMs;
  unsigned long _lastSyncAttemptMs;
  unsigned long _syncIntervalMs;

  bool sendNtpPacket();
  bool readNtpResponse(uint32_t &epoch);
  int16_t currentOffsetMinutes(uint32_t utcEpochValue) const;
  bool isDstUtc(uint32_t utcEpochValue) const;
  void localTm(tm &out) const;

  static int16_t offsetToMinutes(int16_t offset, OffsetUnit unit);
  static uint8_t dayOfWeek(uint16_t year, uint8_t month, uint8_t day);
  static uint8_t lastSunday(uint16_t year, uint8_t month);
  static uint32_t epochFromUtc(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);
  static uint32_t euDstStartUtc(uint16_t year);
  static uint32_t euDstEndUtc(uint16_t year);
  static void fillTm(uint32_t epoch, tm &out);
};

#endif
