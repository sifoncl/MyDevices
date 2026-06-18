#include "InternetClock.h"

InternetClock::InternetClock(WiFiUDP &udp, int16_t utcOffset, OffsetUnit unit, bool autoDst, const char *ntpServer)
    : _udp(udp),
      _ntpServer(ntpServer),
      _baseUtcOffsetMinutes(offsetToMinutes(utcOffset, unit)),
      _autoDst(autoDst),
      _udpStarted(false),
      _synced(false),
      _localPort(2390),
      _utcEpochAtSync(0),
      _syncedAtMs(0),
      _lastSyncAttemptMs(0),
      _syncIntervalMs(DEFAULT_SYNC_INTERVAL_MS)
{
}

InternetClock::InternetClock(WiFiUDP &udp, int8_t utcOffsetHours, bool autoDst, const char *ntpServer)
    : InternetClock(udp, static_cast<int16_t>(utcOffsetHours), OffsetUnit::Hours, autoDst, ntpServer)
{
}

bool InternetClock::begin(uint16_t localPort)
{
  _localPort = localPort;

  if (!_udpStarted)
  {
    _udpStarted = _udp.begin(_localPort) != 0;
  }

  return syncNow();
}

void InternetClock::loop()
{
  const unsigned long nowMs = millis();
  const unsigned long interval = _synced ? _syncIntervalMs : RETRY_INTERVAL_MS;

  if ((nowMs - _lastSyncAttemptMs) >= interval)
  {
    syncNow();
  }
}

bool InternetClock::syncNow()
{
  _lastSyncAttemptMs = millis();

  if (!_udpStarted)
  {
    _udpStarted = _udp.begin(_localPort) != 0;
    if (!_udpStarted)
    {
      return false;
    }
  }

  while (_udp.parsePacket() > 0)
  {
    while (_udp.available() > 0)
    {
      _udp.read();
    }
  }

  if (!sendNtpPacket())
  {
    return false;
  }

  uint32_t epoch = 0;
  if (!readNtpResponse(epoch))
  {
    return false;
  }

  _utcEpochAtSync = epoch;
  _syncedAtMs = millis();
  _synced = true;
  return true;
}

bool InternetClock::isSynced() const
{
  return _synced;
}

bool InternetClock::isDst() const
{
  return _synced && isDstUtc(utcEpoch());
}

void InternetClock::setNtpServer(const char *ntpServer)
{
  if (ntpServer != nullptr)
  {
    _ntpServer = ntpServer;
  }
}

void InternetClock::setSyncInterval(unsigned long intervalMs)
{
  _syncIntervalMs = intervalMs < 1000UL ? 1000UL : intervalMs;
}

void InternetClock::setAutoDst(bool enabled)
{
  _autoDst = enabled;
}

void InternetClock::setBaseUtcOffset(int16_t utcOffset, OffsetUnit unit)
{
  _baseUtcOffsetMinutes = offsetToMinutes(utcOffset, unit);
}

uint32_t InternetClock::utcEpoch() const
{
  if (!_synced)
  {
    return 0;
  }

  return _utcEpochAtSync + static_cast<uint32_t>((millis() - _syncedAtMs) / 1000UL);
}

uint32_t InternetClock::localEpoch() const
{
  if (!_synced)
  {
    return 0;
  }

  const uint32_t utc = utcEpoch();
  const int64_t local = static_cast<int64_t>(utc) + static_cast<int64_t>(currentOffsetMinutes(utc)) * 60LL;
  return static_cast<uint32_t>(local);
}

InternetClockDateTime InternetClock::now() const
{
  InternetClockDateTime dateTime = {0, 0, 0, 0, 0, 0, 0, false, _baseUtcOffsetMinutes};

  if (!_synced)
  {
    return dateTime;
  }

  const uint32_t utc = utcEpoch();
  const int16_t offsetMinutes = currentOffsetMinutes(utc);
  const int64_t local = static_cast<int64_t>(utc) + static_cast<int64_t>(offsetMinutes) * 60LL;

  tm localInfo;
  fillTm(static_cast<uint32_t>(local), localInfo);

  dateTime.year = static_cast<uint16_t>(localInfo.tm_year + 1900);
  dateTime.month = static_cast<uint8_t>(localInfo.tm_mon + 1);
  dateTime.day = static_cast<uint8_t>(localInfo.tm_mday);
  dateTime.hour = static_cast<uint8_t>(localInfo.tm_hour);
  dateTime.minute = static_cast<uint8_t>(localInfo.tm_min);
  dateTime.second = static_cast<uint8_t>(localInfo.tm_sec);
  dateTime.weekDay = static_cast<uint8_t>(localInfo.tm_wday);
  dateTime.isDst = _autoDst && isDstUtc(utc);
  dateTime.utcOffsetMinutes = offsetMinutes;
  return dateTime;
}

uint16_t InternetClock::year() const
{
  return now().year;
}

uint8_t InternetClock::month() const
{
  return now().month;
}

uint8_t InternetClock::day() const
{
  return now().day;
}

uint8_t InternetClock::hour() const
{
  return now().hour;
}

uint8_t InternetClock::minute() const
{
  return now().minute;
}

uint8_t InternetClock::second() const
{
  return now().second;
}

uint8_t InternetClock::weekDay() const
{
  return now().weekDay;
}

String InternetClock::timeString() const
{
  return _synced ? format("%H:%M:%S") : String("--:--:--");
}

String InternetClock::shortTimeString() const
{
  return _synced ? format("%H:%M") : String("--:--");
}

String InternetClock::dateString() const
{
  return _synced ? format("%Y-%m-%d") : String();
}

String InternetClock::dateTimeString() const
{
  return _synced ? format("%Y-%m-%d %H:%M:%S") : String();
}

String InternetClock::isoString() const
{
  if (!_synced)
  {
    return String();
  }

  const InternetClockDateTime dateTime = now();
  const int16_t offsetMinutes = dateTime.utcOffsetMinutes;
  const char sign = offsetMinutes >= 0 ? '+' : '-';
  const uint16_t absoluteOffset = offsetMinutes >= 0 ? offsetMinutes : -offsetMinutes;

  char buffer[32];
  snprintf(buffer,
           sizeof(buffer),
           "%04u-%02u-%02uT%02u:%02u:%02u%c%02u:%02u",
           dateTime.year,
           dateTime.month,
           dateTime.day,
           dateTime.hour,
           dateTime.minute,
           dateTime.second,
           sign,
           absoluteOffset / 60,
           absoluteOffset % 60);
  return String(buffer);
}

String InternetClock::format(const char *pattern) const
{
  char buffer[64];
  if (!formatTo(buffer, sizeof(buffer), pattern))
  {
    return String();
  }

  return String(buffer);
}

bool InternetClock::formatTo(char *buffer, size_t length, const char *pattern) const
{
  if (buffer == nullptr || length == 0)
  {
    return false;
  }

  buffer[0] = '\0';

  if (!_synced || pattern == nullptr)
  {
    return false;
  }

  tm localInfo;
  localTm(localInfo);
  return strftime(buffer, length, pattern, &localInfo) > 0;
}

bool InternetClock::sendNtpPacket()
{
  if (_ntpServer == nullptr)
  {
    return false;
  }

  uint8_t packetBuffer[NTP_PACKET_SIZE] = {0};
  packetBuffer[0] = 0x1B; // LI = 0, version = 3, mode = client.

  if (_udp.beginPacket(_ntpServer, NTP_PORT) == 0)
  {
    return false;
  }

  _udp.write(packetBuffer, NTP_PACKET_SIZE);
  return _udp.endPacket() != 0;
}

bool InternetClock::readNtpResponse(uint32_t &epoch)
{
  const unsigned long startedAt = millis();

  while ((millis() - startedAt) < NTP_TIMEOUT_MS)
  {
    const int packetSize = _udp.parsePacket();
    if (packetSize >= static_cast<int>(NTP_PACKET_SIZE))
    {
      uint8_t packetBuffer[NTP_PACKET_SIZE];
      _udp.read(packetBuffer, NTP_PACKET_SIZE);

      const uint32_t secondsSince1900 =
          (static_cast<uint32_t>(packetBuffer[40]) << 24) |
          (static_cast<uint32_t>(packetBuffer[41]) << 16) |
          (static_cast<uint32_t>(packetBuffer[42]) << 8) |
          static_cast<uint32_t>(packetBuffer[43]);

      if (secondsSince1900 < NTP_UNIX_OFFSET)
      {
        return false;
      }

      epoch = secondsSince1900 - NTP_UNIX_OFFSET;
      return true;
    }

    delay(10);
  }

  return false;
}

int16_t InternetClock::currentOffsetMinutes(uint32_t utcEpochValue) const
{
  return _baseUtcOffsetMinutes + (isDstUtc(utcEpochValue) ? 60 : 0);
}

bool InternetClock::isDstUtc(uint32_t utcEpochValue) const
{
  if (!_autoDst)
  {
    return false;
  }

  tm utcInfo;
  fillTm(utcEpochValue, utcInfo);
  const uint16_t currentYear = static_cast<uint16_t>(utcInfo.tm_year + 1900);
  return utcEpochValue >= euDstStartUtc(currentYear) && utcEpochValue < euDstEndUtc(currentYear);
}

void InternetClock::localTm(tm &out) const
{
  const uint32_t utc = utcEpoch();
  const int64_t local = static_cast<int64_t>(utc) + static_cast<int64_t>(currentOffsetMinutes(utc)) * 60LL;
  fillTm(static_cast<uint32_t>(local), out);
  out.tm_isdst = isDstUtc(utc) ? 1 : 0;
}

int16_t InternetClock::offsetToMinutes(int16_t offset, OffsetUnit unit)
{
  return unit == OffsetUnit::Hours ? static_cast<int16_t>(offset * 60) : offset;
}

uint8_t InternetClock::dayOfWeek(uint16_t year, uint8_t month, uint8_t day)
{
  static const uint8_t monthOffsets[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
  uint16_t adjustedYear = year;

  if (month < 3)
  {
    adjustedYear--;
  }

  return static_cast<uint8_t>((adjustedYear + adjustedYear / 4 - adjustedYear / 100 + adjustedYear / 400 + monthOffsets[month - 1] + day) % 7);
}

uint8_t InternetClock::lastSunday(uint16_t year, uint8_t month)
{
  const uint8_t lastDay = 31;
  return static_cast<uint8_t>(lastDay - dayOfWeek(year, month, lastDay));
}

uint32_t InternetClock::epochFromUtc(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second)
{
  int y = year;
  const unsigned m = month;
  y -= m <= 2;

  const int era = (y >= 0 ? y : y - 399) / 400;
  const unsigned yoe = static_cast<unsigned>(y - era * 400);
  const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + day - 1;
  const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  const int days = era * 146097 + static_cast<int>(doe) - 719468;

  return static_cast<uint32_t>(static_cast<int64_t>(days) * 86400LL +
                               static_cast<int64_t>(hour) * 3600LL +
                               static_cast<int64_t>(minute) * 60LL +
                               second);
}

uint32_t InternetClock::euDstStartUtc(uint16_t year)
{
  return epochFromUtc(year, 3, lastSunday(year, 3), 1, 0, 0);
}

uint32_t InternetClock::euDstEndUtc(uint16_t year)
{
  return epochFromUtc(year, 10, lastSunday(year, 10), 1, 0, 0);
}

void InternetClock::fillTm(uint32_t epoch, tm &out)
{
  const time_t rawTime = static_cast<time_t>(epoch);
  gmtime_r(&rawTime, &out);
}
