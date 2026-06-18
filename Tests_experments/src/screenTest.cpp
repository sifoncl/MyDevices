#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define WIDTH 24
#define HEIGHT 60
#define BYTES_PER_ROW WIDTH/8

void printHexArray(uint8_t *data, int size, int elementsPerLine = 5)
{
    Serial.print("const unsigned char bitmap[] = { // Size 40x60 pixels");
    Serial.println();

    for (int i = 0; i < size; i++)
    {
        // Начало строки с отступом
        if (i % elementsPerLine == 0)
        {
            Serial.print("    ");
        }

        // Вывод элемента в hex формате
        if (data[i] < 0x10)
        {
            Serial.print("0x0"); // Добавляем ведущий ноль для значений меньше 0x10
            Serial.print(data[i], HEX);
        }
        else
        {
            Serial.print("0x");
            Serial.print(data[i], HEX);
        }

        // Добавляем запятую (кроме последнего элемента)
        if (i < size - 1)
        {
            Serial.print(", ");
        }

        // Переход на новую строку после elementsPerLine элементов
        if ((i + 1) % elementsPerLine == 0 && i < size - 1)
        {
            Serial.println();
        }
    }
    Serial.println();
    Serial.println("};");
}

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// unsigned char bitmap[] = {
//     // Size 40x60 pixels
//     0x00, 0x00, 0x00, 0x00, 0x00,
//     0x00, 0x00, 0x20, 0x00, 0x00,
//     0x00, 0x00, 0x30, 0x00, 0x00,
//     0x00, 0x00, 0x70, 0x00, 0x00,
//     0x00, 0x00, 0x70, 0x00, 0x00,
//     0x00, 0x00, 0x78, 0x00, 0x00,
//     0x00, 0x00, 0xf8, 0x00, 0x00,
//     0x00, 0x00, 0xfc, 0x00, 0x00,
//     0x00, 0x01, 0xfc, 0x00, 0x00,
//     0x00, 0x01, 0xce, 0x00, 0x00,
//     0x00, 0x03, 0x8e, 0x00, 0x00,
//     0x00, 0x03, 0x87, 0x00, 0x00,
//     0x00, 0x07, 0x07, 0x00, 0x00,
//     0x00, 0x0f, 0x03, 0x80, 0x00,
//     0x00, 0x0e, 0x03, 0xc0, 0x00,
//     0x00, 0x1e, 0x01, 0xc0, 0x00,
//     0x00, 0x1c, 0x01, 0xe0, 0x00,
//     0x00, 0x3c, 0x00, 0xe0, 0x00,
//     0x00, 0x38, 0x00, 0xf0, 0x00,
//     0x00, 0x70, 0x00, 0x70, 0x00,
//     0x00, 0xf0, 0x00, 0x38, 0x00,
//     0x00, 0xe0, 0x00, 0x3c, 0x00,
//     0x01, 0xe0, 0x00, 0x1c, 0x00,
//     0x01, 0xc0, 0x00, 0x0e, 0x00,
//     0x03, 0x80, 0x00, 0x0e, 0x00,
//     0x07, 0x80, 0x00, 0x07, 0x00,
//     0x07, 0x00, 0x00, 0x07, 0x80,
//     0x0f, 0x00, 0x00, 0x03, 0x80,
//     0x0e, 0x00, 0x00, 0x03, 0xc0,
//     0x1e, 0x00, 0x00, 0x01, 0xc0,
//     0x1c, 0x00, 0x00, 0x01, 0xe0,
//     0x1c, 0x00, 0x00, 0x00, 0xe0,
//     0x38, 0x00, 0x00, 0x00, 0xe0,
//     0x38, 0x00, 0x00, 0x00, 0xf0,
//     0x38, 0x00, 0x00, 0x00, 0x70,
//     0x70, 0x00, 0x00, 0x00, 0x70,
//     0x70, 0x00, 0x00, 0x00, 0x78,
//     0x70, 0x00, 0x00, 0x00, 0x38,
//     0xf0, 0x00, 0x00, 0x00, 0x38,
//     0xf0, 0x00, 0x00, 0x00, 0x38,
//     0xf0, 0x00, 0x00, 0x00, 0x38,
//     0xf0, 0x00, 0x00, 0x00, 0x38,
//     0xf0, 0x00, 0x00, 0x00, 0x38,
//     0xf0, 0x00, 0x00, 0x00, 0x38,
//     0x70, 0x00, 0x00, 0x00, 0x38,
//     0x70, 0x00, 0x00, 0x00, 0x78,
//     0x70, 0x00, 0x00, 0x00, 0x70,
//     0x38, 0x00, 0x00, 0x00, 0x70,
//     0x38, 0x00, 0x00, 0x00, 0xf0,
//     0x3c, 0x00, 0x00, 0x00, 0xe0,
//     0x1c, 0x00, 0x00, 0x01, 0xe0,
//     0x1e, 0x00, 0x00, 0x03, 0xc0,
//     0x0f, 0x00, 0x00, 0x03, 0x80,
//     0x07, 0x80, 0x00, 0x0f, 0x80,
//     0x03, 0xe0, 0x00, 0x1f, 0x00,
//     0x01, 0xf0, 0x00, 0x7e, 0x00,
//     0x00, 0xfe, 0x03, 0xf8, 0x00,
//     0x00, 0x3f, 0xff, 0xf0, 0x00,
//     0x00, 0x0f, 0xff, 0xc0, 0x00,
//     0x00, 0x03, 0xfe, 0x00, 0x00};



const unsigned char bitmap[] = { // Size 24x60 pixels
  0x01, 0xf0, 0x00,
    0x07, 0xfc, 0x00,
    0x0e, 0x0c, 0x00,
    0x0c, 0x06, 0x00,
    0x08, 0x02, 0x00,
    0x18, 0x03, 0x00,
    0x18, 0x03, 0x00,
    0x18, 0x03, 0x00,
    0x18, 0x03, 0x00,
    0x18, 0x03, 0x00,
    0x18, 0x03, 0x00,
    0x18, 0x03, 0x00,
    0x18, 0x03, 0x00,
    0x18, 0x03, 0x00,
    0x18, 0x03, 0x00,
    0x18, 0x03, 0x00,
    0x18, 0x03, 0x00,
    0x18, 0x03, 0x00,
    0x18, 0x03, 0x00,
    0x18, 0x03, 0x00,
    0x18, 0x03, 0x00,
    0x18, 0x03, 0x00,
    0x18, 0x03, 0x00,
    0x18, 0x03, 0x00,
    0x18, 0x03, 0x00,
    0x18, 0x03, 0x00,
    0x18, 0x03, 0x00,
    0x18, 0x03, 0x00,
    0x18, 0x03, 0x00,
    0x18, 0x03, 0x00,
    0x18, 0x03, 0x00,
    0x18, 0x03, 0x00,
    0x18, 0x03, 0x00,
    0x18, 0x03, 0x00,
    0x18, 0x03, 0x00,
    0x18, 0x03, 0x00,
    0x18, 0x03, 0x00,
    0x18, 0x03, 0x00,
    0x18, 0x03, 0x00,
    0x18, 0x03, 0x00,
    0x18, 0x03, 0x00,
    0x18, 0x03, 0x00,
    0x18, 0x03, 0x00,
    0x18, 0x03, 0x00,
    0x3f, 0xff, 0x80,
    0x3f, 0xff, 0x80,
    0x7f, 0xff, 0xc0,
    0xff, 0xff, 0xc0,
    0xff, 0xff, 0xe0,
    0xff, 0xff, 0xe0,
    0xff, 0xff, 0xe0,
    0xff, 0xff, 0xe0,
    0xff, 0xff, 0xe0,
    0xff, 0xff, 0xe0,
    0x7f, 0xff, 0xc0,
    0x7f, 0xff, 0xc0,
    0x3f, 0xff, 0x80,
    0x1f, 0xff, 0x00,
    0x0f, 0xfe, 0x00,
    0x03, 0xf8, 0x00
};


unsigned char work_bitmap[sizeof(bitmap) + 1];

void init_work_bitmap()
{
    // Копируем побайтно для безопасности
    for (int i = 0; i < sizeof(bitmap); i++)
    {
        work_bitmap[i] = bitmap[i];
    }
    Serial.printf("Рабочий массив инициализирован из оригинала.\n");
}

typedef union
{
    unsigned char value;
    struct
    {
        unsigned char b0 : 1;
        unsigned char b1 : 1;
        unsigned char b2 : 1;
        unsigned char b3 : 1;
        unsigned char b4 : 1;
        unsigned char b5 : 1;
        unsigned char b6 : 1;
        unsigned char b7 : 1;
    } bits;
} byte_union;

void setup()
{
    Serial.begin(115200);

    init_work_bitmap();

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    { // Address 0x3D for 128x64
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ;
    }
    delay(2000);
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    // Display static text
    display.println("Scrolling Hello");
    display.display();
    delay(100);
    //   display.invertDisplay(1);
    display.clearDisplay();
}
void set_bit(byte_union *bytes, int x, int y, bool value)
{
    int byte_index = y * BYTES_PER_ROW + (x / 8);
    int bit_position = x % 8;

    // Устанавливаем нужный бит
    unsigned char bit_value = value ? 1 : 0;

    switch (bit_position)
    {
    case 0:
        bytes[byte_index].bits.b7 = bit_value;
        break;
    case 1:
        bytes[byte_index].bits.b6 = bit_value;
        break;
    case 2:
        bytes[byte_index].bits.b5 = bit_value;
        break;
    case 3:
        bytes[byte_index].bits.b4 = bit_value;
        break;
    case 4:
        bytes[byte_index].bits.b3 = bit_value;
        break;
    case 5:
        bytes[byte_index].bits.b2 = bit_value;
        break;
    case 6:
        bytes[byte_index].bits.b1 = bit_value;
        break;
    case 7:
        bytes[byte_index].bits.b0 = bit_value;
        break;
    }
}

bool get_bit(byte_union *bytes, int x, int y)
{
    int byte_index = y * BYTES_PER_ROW + (x / 8);
    int bit_position = x % 8;

    // Получаем нужный байт
    byte_union current_byte = bytes[byte_index];

    // Возвращаем нужный бит как bool
    switch (bit_position)
    {
    case 0:
        return current_byte.bits.b7 == 1; // Старший бит
    case 1:
        return current_byte.bits.b6 == 1;
    case 2:
        return current_byte.bits.b5 == 1;
    case 3:
        return current_byte.bits.b4 == 1;
    case 4:
        return current_byte.bits.b3 == 1;
    case 5:
        return current_byte.bits.b2 == 1;
    case 6:
        return current_byte.bits.b1 == 1;
    case 7:
        return current_byte.bits.b0 == 1; // Младший бит
    default:
        return false;
    }
}

void print_bitmap(byte_union *bytes)
{
    Serial.printf("Изображение %dx%d пикселей:\n", WIDTH, HEIGHT);
    Serial.printf("┌────────────────────────────────────────┐\n");

    for (int y = 0; y < HEIGHT; y++)
    {
        Serial.printf("│");
        for (int x = 0; x < WIDTH; x++)
        {
            bool pixel = get_bit(bytes, x, y);
            Serial.printf("%c", pixel ? '█' : ' '); // true = черный, false = белый
        }
        Serial.printf("│\n");
    }

    Serial.printf("└────────────────────────────────────────┘\n");
}

void fill_all_between_boundaries_in_row(byte_union *bytes, int y)
{
    // Находим все граничные позиции в строке
    int boundaries[WIDTH];
    int boundary_count = 0;

    // Собираем позиции всех границ (единиц)
    for (int x = 0; x < WIDTH; x++)
    {
        if (get_bit(bytes, x, y))
        {
            boundaries[boundary_count++] = x;
        }
    }

    // Если есть хотя бы 2 границы, заливаем между ними
    for (int i = 0; i < boundary_count - 1; i++)
    {
        int start = boundaries[i];
        int end = boundaries[i + 1];

        // Заливаем все между start и end
        for (int x = start + 1; x < end; x++)
        {
            set_bit(bytes, x, y, true);
        }
    }
}

// Основная функция заливки до определенной высоты
void fill_image_to_height(byte_union *bytes, int max_height)
{
    if (max_height > HEIGHT)
    {
        max_height = HEIGHT;
    }
    


// Serial.printf("Заливка изображения до высоты %d...\n", max_height);

for (int y = HEIGHT-1; y > max_height; y--)
{
    // Serial.printf("Заливаю слой %d \n", y);
    fill_all_between_boundaries_in_row(bytes, y);
}
}
void reset_to_original()
{
    // Копируем побайтно для безопасности
    for (int i = 0; i < sizeof(bitmap); i++)
    {
        work_bitmap[i] = bitmap[i];
    }
    // Serial.printf("Рабочий массив сброшен к оригиналу.\n");
}
 int filled_rows = 1;
void loop()
{
    // Serial.println(filled_rows);
    display.clearDisplay();
    byte_union *bytes = (byte_union *)work_bitmap;
    boolean bit_value = bytes[0].bits.b7;

    fill_image_to_height(bytes, HEIGHT-filled_rows);
    //   fill_all_between_boundaries_in_row(bytes, 50);

    print_bitmap(bytes);

    // printHexArray(bitmap, sizeof(bitmap) / sizeof(bitmap[0]));

    display.drawBitmap(0, 0, work_bitmap, WIDTH, HEIGHT, WHITE);
    display.display();
    reset_to_original();
    // delay(10);
    //  display.clearDisplay();
    //  display.drawBitmap(0, 0, work_bitmap, 40, 60, WHITE);
    //  display.display();
    //  delay(10000);
    filled_rows++;
    if (filled_rows == 60)
    {
        filled_rows = 1;
        reset_to_original();
        display.clearDisplay();
    }
}