# Banya Thermometer

Термометр для ESP32-S3 Super Mini, датчика SHT85 и четырех матриц MAX7219.

Профиль платы и параметры памяти соответствуют проекту `Kid Room tem`.

## Подключение

| Устройство | Сигнал | GPIO |
|---|---|---:|
| SHT85 | SDA | 5 |
| SHT85 | SCL | 3 |
| MAX7219 | DIN | 13 |
| MAX7219 | CLK | 11 |
| MAX7219 | CS/LOAD | 12 |

У всех модулей должна быть общая земля. GPIO ESP32-S3 работают с логикой 3.3 В и не допускают подачу 5 В на входы.

MAX7219 обычно питается от 5 В. Если модуль нестабильно воспринимает сигнал 3.3 В, используйте преобразователь уровней 74HCT125/74AHCT125 для DIN, CLK и CS.

Ориентация матриц настраивается в `src/AppConfig.h`: `MATRIX_REVERSE_SEGMENT_ORDER`, `MATRIX_MIRROR_COLUMNS` и `MATRIX_MIRROR_ROWS`.

## Сеть

- Заводские параметры находятся в `src/AppConfig.h`.
- DHCP hostname: `banya-thermometer`.
- Точка настройки: `BanyaTermometr-Setup`.
- Страница настройки: `http://192.168.4.1/`.
- Web UI поддерживает DHCP и статический IPv4.
- Настройки сохраняются во flash через `Preferences`.

## Сборка

```powershell
C:\Users\virus\.platformio\penv\Scripts\platformio.exe run
```
