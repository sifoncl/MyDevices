// RGBWWWController.h
#ifndef RGBWWWCONTROLLER_H // Защита от повторного включения
#define RGBWWWCONTROLLER_H

#include <Arduino.h> // Подключаем только здесь

class RGBWWWController
{
private:
    // Цветовые компоненты
    uint8_t m_r = 0;
    uint8_t m_g = 0;
    uint8_t m_b = 0;
    uint8_t m_cw = 0;
    uint8_t m_ww = 0;
    uint8_t m_brightness = 0;

    // Пины для управления
    uint8_t m_pinR;
    uint8_t m_pinG;
    uint8_t m_pinB;
    uint8_t m_pinCW;
    uint8_t m_pinWW;

    uint16_t m_color_temp = 320;

    bool m_is_ww_state = true;
    bool m_is_on = false;

    uint16_t m_max_temp = 454;
    uint16_t m_min_temp = 153;
    bool m_is_mireds = true;
    uint32_t m_revision = 0;

    /**
     * Инициализация пинов
     * Приватный метод - вызывается только в конструкторе
     */
    void init();

    /**
     * Обновление всех каналов согласно текущему режиму
     */
    void updateLight();

public:
    /**
     * Конструктор контроллера
     * @param pinR Пин для красного канала
     * @param pinG Пин для зеленого канала
     * @param pinB Пин для синего канала
     * @param pinW Пин для холодного белого (White)
     * @param pinWW Пин для теплого белого (Warm White)
     */
    RGBWWWController(uint8_t pinR, uint8_t pinG, uint8_t pinB,
                     uint8_t pinCW, uint8_t pinWW);

    /**
     * Установка всех параметров сразу
     * @param r Красный (0-255)
     * @param g Зеленый (0-255)
     * @param b Синий (0-255)
     * @param w Холодный белый (0-255)
     * @param ww Теплый белый (0-255)
     * @param brightness Яркость (0-255)
     */
    void setRGBWW(uint8_t r, uint8_t g, uint8_t b,
                  uint8_t cw, uint8_t ww, uint8_t brightness);

    /**
     * Установка только RGB компонентов
     * @param r Красный (0-255)
     * @param g Зеленый (0-255)
     * @param b Синий (0-255)
     * @param brightness Яркость (0-255)
     */
    void setRGB(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness);

    /**
     * Установка только белых компонентов
     * @param w Холодный белый (0-255)
     * @param ww Теплый белый (0-255)
     * @param brightness Яркость (0-255)
     */
    void setWhite(uint8_t w, uint8_t ww, uint8_t brightness);

    /**
     * Установка только яркости
     * @param brightness Яркость (0-255)
     */
    void setBrightness(uint8_t brightness);

    /**
     * Плавное изменение яркости
     * @param targetBrightness Целевая яркость (0-255)
     * @param duration Время анимации в миллисекундах
     */
    void fadeBrightness(uint8_t targetBrightness, uint16_t duration = 1000);

    /**
     * Плавный переход к цвету
     * @param r Красный (0-255)
     * @param g Зеленый (0-255)
     * @param b Синий (0-255)
     * @param w Холодный белый (0-255)
     * @param ww Теплый белый (0-255)
     * @param brightness Яркость (0-255)
     * @param duration Время анимации в миллисекундах
     */
    void fadeToColor(uint8_t r, uint8_t g, uint8_t b,
                     uint8_t cw, uint8_t ww,
                     uint8_t brightness, uint16_t duration = 1000);

    // Геттеры для получения текущих значений
    uint8_t getRed() const { return m_r; }
    uint8_t getGreen() const { return m_g; }
    uint8_t getBlue() const { return m_b; }
    uint8_t getWhite() const { return m_cw; }
    uint8_t getWarmWhite() const { return m_ww; }
    uint8_t getBrightness() const { return m_brightness; }
    uint16_t getColorTemperature() const { return m_color_temp; }
    bool isWhiteMode() const { return m_is_ww_state; }
    uint32_t revision() const { return m_revision; }

    /**
     * Выключить все светодиоды
     */
    void turnOff();

    /**
     * Включить все светодиоды на максимальную яркость (белый)
     */
    void turnOn();

    /**
     * Проверка, включен ли свет
     * @return true если хотя бы один канал не нулевой
     */
    bool isOn();

    void setWWCWmode();

    void setRGBmode();

    void setColorTempAndChangeMode(int colorTemp);

    void setRGBandChageMode(uint8_t r, uint8_t g, uint8_t b);

    void setTempParams(uint16_t max_temp, uint16_t min_temp, bool isMireds);

    ~RGBWWWController();
};

#endif // RGBWWWCONTROLLER_H
