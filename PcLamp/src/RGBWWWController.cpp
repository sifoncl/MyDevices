#include "RGBWWWController.h"

void RGBWWWController::init()
{
    pinMode(m_pinR, OUTPUT);
    pinMode(m_pinG, OUTPUT);
    pinMode(m_pinB, OUTPUT);
    pinMode(m_pinCW, OUTPUT);
    pinMode(m_pinWW, OUTPUT);

    // Убедимся, что все выключены при старте
    analogWrite(m_pinR, 0);
    analogWrite(m_pinG, 0);
    analogWrite(m_pinB, 0);
    analogWrite(m_pinCW, 0);
    analogWrite(m_pinWW, 0);
}

// Конструктор
RGBWWWController::RGBWWWController(uint8_t pinR, uint8_t pinG, uint8_t pinB,
                                   uint8_t pinCW, uint8_t pinWW)
    : m_pinR(pinR), m_pinG(pinG), m_pinB(pinB),
      m_pinCW(pinCW), m_pinWW(pinWW)
{
    init();
}

/**
 * Вспомогательная функция для конвертации Кельвинов в mireds
 * Mireds = 1,000,000 / Кельвины
 */
int kelvinToMireds(int kelvin)
{
    if (kelvin <= 0)
        return 454; // Защита от деления на ноль
    return 1000000 / kelvin;
}

/**
 * Версия с учетом нелинейности восприятия цвета (использует гамма-коррекцию)
 * Принимает цветовую температуру в Кельвинах (рекомендуемый диапазон 2200K-6500K)
 */
void calculateWhiteChannelsGamma(int brightness, int kelvin, uint8_t &warm_white, uint8_t &cold_white, u_int16_t m_min_temp, u_int16_t m_max_temp, float gamma = 2.2)
{
    // Ограничиваем входные значения
    brightness = constrain(brightness, 0, 255);

    // Ограничиваем температуру в Кельвинах
    kelvin = constrain(kelvin, m_min_temp, m_max_temp);

    // Нормализуем цветовую температуру (от 0 до 1)
    // 0 = максимально теплый (min_kelvin)
    // 1 = максимально холодный (max_kelvin)
    float temp_norm = (float)(kelvin - m_min_temp) / (m_max_temp - m_min_temp);

    // Используем синусоидальную кривую для более естественного перехода
    float warm_coeff = (cos(temp_norm * PI) + 1.0) / 2.0;
    float cold_coeff = 1.0 - warm_coeff;

    // Применяем гамма-коррекцию для более естественного восприятия
    warm_coeff = pow(warm_coeff, 1.0 / gamma);
    cold_coeff = pow(cold_coeff, 1.0 / gamma);

    // Нормализуем коэффициенты
    float total = warm_coeff + cold_coeff;
    if (total > 0)
    {
        warm_coeff /= total;
        cold_coeff /= total;
    }

    // Применяем яркость
    warm_white = (int)(brightness * warm_coeff);
    cold_white = (int)(brightness * cold_coeff);

    // Гарантируем пределы
    warm_white = constrain(warm_white, 0, 255);
    cold_white = constrain(cold_white, 0, 255);
}

int miredsToKelvin(int mireds);

/**
 * Версия с учетом нелинейности восприятия цвета (использует гамма-коррекцию)
 * Принимает цветовую температуру в mireds (микрообратных кельвинах)
 * Рекомендуемый диапазон: 153 mireds (6500K) - 454 mireds (2200K)
 */
void calculateWhiteChannelsMireds(int brightness, int mireds, uint8_t &warm_white, uint8_t &cold_white,
                                  u_int16_t m_min_temp = 153, u_int16_t m_max_temp = 454, float gamma = 2.2)
{
    warm_white = miredsToKelvin(warm_white);

    // Ограничиваем mireds в допустимом диапазоне
    if (mireds < m_min_temp)
        mireds = m_min_temp;
    if (mireds > m_max_temp)
        mireds = m_max_temp;

    // Нормализуем температуру в диапазоне 0-1 (0 - холодный, 1 - теплый)
    float temp_normalized = (float)(mireds - m_min_temp) / (m_max_temp - m_min_temp);

    // Линейные значения для каналов
    float warm_linear = temp_normalized;        // Теплый белый: от 0 до 1
    float cold_linear = 1.0f - temp_normalized; // Холодный белый: от 1 до 0

    // Применяем гамма-коррекцию
    float warm_gamma = pow(warm_linear, 1.0f / gamma);
    float cold_gamma = pow(cold_linear, 1.0f / gamma);

    // Масштабируем с учетом яркости
    float bright_scale = (float)brightness / 255.0f;
    float warm_scaled = warm_gamma * bright_scale;
    float cold_scaled = cold_gamma * bright_scale;

    // // Нормализуем, чтобы сумма каналов не превышала brightness
    // // (это предотвращает перенасыщение при смешивании)
    // float sum = warm_scaled + cold_scaled;
    // if (sum > 1.0f && sum > 0.0f) {
    //     warm_scaled /= sum;
    //     cold_scaled /= sum;
    // }

    // Преобразуем в значения ШИМ 0-255
    warm_white = (uint8_t)(warm_scaled * 255.0f);
    cold_white = (uint8_t)(cold_scaled * 255.0f);
}

/**
 * Вспомогательная функция для конвертации mireds в Кельвины
 * Кельвины = 1,000,000 / Mireds
 */
int miredsToKelvin(int mireds)
{
    if (mireds <= 0)
        return 6500; // Защита от деления на ноль
    return 1000000 / mireds;
}

// Обновление RGB каналов
void RGBWWWController::updateLight()
{
    Serial.println("Обновление состояния ");
    // Управляем светодиодами в зависимости от состояния
    if (m_is_on)
    {
        Serial.println("Лента включена ");
        if (m_is_ww_state)
        {
            Serial.println("Режим WWCW");

            if (m_is_mireds)
            {
                calculateWhiteChannelsMireds(m_brightness, m_color_temp, m_ww, m_cw, m_min_temp, m_max_temp);
            }
            else
            {
                calculateWhiteChannelsGamma(m_brightness, m_color_temp, m_ww, m_cw, m_min_temp, m_max_temp);
            }

            Serial.print("m_brightness: ");
            Serial.println(m_brightness);

            Serial.print("cold_white: ");
            Serial.println(m_cw);
            Serial.print("warm_white: ");
            Serial.println(m_ww);
            analogWrite(m_pinR, 0);
            analogWrite(m_pinG, 0);
            analogWrite(m_pinB, 0);
            analogWrite(m_pinWW, m_ww);
            analogWrite(m_pinCW, m_cw);
        }
        else
        {
            Serial.println("Режим RGB");

            Serial.println("m_brightness: ");
            Serial.print(m_brightness);

            Serial.print("r: ");
            Serial.println(m_r);
            Serial.print("g: ");
            Serial.print(m_g);
            Serial.print("b: ");
            Serial.println(m_b);

            // RGB режим
            analogWrite(m_pinR, m_r * m_brightness / 255);
            analogWrite(m_pinG, m_g * m_brightness / 255);
            analogWrite(m_pinB, m_b * m_brightness / 255);
            analogWrite(m_pinWW, 0);
            analogWrite(m_pinCW, 0);
        }
    }
    else
    {
        Serial.println("Лента выключена");
        // Все выключено
        analogWrite(m_pinR, 0);
        analogWrite(m_pinG, 0);
        analogWrite(m_pinB, 0);
        analogWrite(m_pinWW, 0);
        analogWrite(m_pinCW, 0);
    }
}

// Установка всех компонентов
void RGBWWWController::setRGBWW(uint8_t r, uint8_t g, uint8_t b,
                                uint8_t w, uint8_t ww, uint8_t brightness)
{
    m_r = r;
    m_g = g;
    m_b = b;
    m_cw = w;
    m_ww = ww;
    m_brightness = brightness;
    updateLight();
}

// Установка RGB
void RGBWWWController::setRGB(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness)
{
    m_r = r;
    m_g = g;
    m_b = b;
    m_brightness = brightness;
    updateLight();
}

// Установка белых каналов
void RGBWWWController::setWhite(uint8_t w, uint8_t ww, uint8_t brightness)
{
    m_cw = w;
    m_ww = ww;
    m_brightness = brightness;
    updateLight();
}

// Установка яркости
void RGBWWWController::setBrightness(uint8_t brightness)
{
    m_brightness = brightness;
    updateLight();
}

// Выключить все
void RGBWWWController::turnOff()
{
    m_is_on = false;
    updateLight();
}
void RGBWWWController::turnOn()
{
    m_is_on = true;
    updateLight();
}

void RGBWWWController::setColorTempAndChangeMode(int color_temp)
{
    m_color_temp = color_temp;
    m_is_ww_state = true;
    updateLight();
}

void RGBWWWController::setRGBandChageMode(uint8_t r, uint8_t g, uint8_t b)
{
    m_is_ww_state = false;
    m_is_on = true;
    setRGB(r, g, b, m_brightness);
}

/**
 * Проверка, включен ли свет
 * @return true если хотя бы один канал не нулевой
 */
bool RGBWWWController::isOn()
{
    return m_is_on;
};

void RGBWWWController::setWWCWmode()
{
    m_is_ww_state = true;
}

void RGBWWWController::setRGBmode()
{
    m_is_ww_state = false;
}

void RGBWWWController::setTempParams(uint16_t max_temp, u_int16_t min_temp, boolean isMireds)
{
    m_max_temp = max_temp;
    m_min_temp = min_temp;
    m_is_mireds = isMireds;
}

// Деструктор
RGBWWWController::~RGBWWWController()
{
    // Выключаем все светодиоды при удалении объекта
    turnOff();
}
