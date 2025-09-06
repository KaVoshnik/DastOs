#include "../include/keyboard.h"
#include "../include/types.h"

// Функции для работы с портами
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Внутренние функции
static void update_modifiers(uint8_t scancode, bool pressed, bool extended);

// Глобальное состояние клавиатуры
static keyboard_state_t kbd_state = {0};
static keyboard_callback_t kbd_callback = NULL;

// Расширенная таблица сканкодов для всех клавиш
static const char scancode_ascii_normal[256] = {
    0,   27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t', // 0x00-0x0F
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,   'a', 's', // 0x10-0x1F
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,   '\\', 'z', 'x', 'c', 'v', // 0x20-0x2F
    'b', 'n', 'm', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,   // 0x30-0x3F
    0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1', // 0x40-0x4F
    '2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   // 0x50-0x5F
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   // 0x60-0x6F
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   // 0x70-0x7F
    // Остальные значения 0x80-0xFF остаются нулями
};

static const char scancode_ascii_shift[256] = {
    0,   27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t', // 0x00-0x0F
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,   'A', 'S', // 0x10-0x1F
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,   '|', 'Z', 'X', 'C', 'V', // 0x20-0x2F
    'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,   // 0x30-0x3F
    0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1', // 0x40-0x4F
    '2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   // 0x50-0x5F
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   // 0x60-0x6F
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   // 0x70-0x7F
    // Остальные значения 0x80-0xFF остаются нулями
};

// Названия клавиш для всех сканкодов
static const char* scancode_names[256] = {
    "NONE", "ESC", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "BACKSPACE", "TAB",
    "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "[", "]", "ENTER", "LCTRL", "A", "S",
    "D", "F", "G", "H", "J", "K", "L", ";", "'", "`", "LSHIFT", "\\", "Z", "X", "C", "V",
    "B", "N", "M", ",", ".", "/", "RSHIFT", "KP*", "LALT", "SPACE", "CAPSLOCK", "F1", "F2", "F3", "F4", "F5",
    "F6", "F7", "F8", "F9", "F10", "NUMLOCK", "SCROLLLOCK", "KP7", "KP8", "KP9", "KP-", "KP4", "KP5", "KP6", "KP+", "KP1",
    "KP2", "KP3", "KP0", "KP.", "SYSRQ", "UNKNOWN", "UNKNOWN", "F11", "F12",
    // Остальные инициализируются как NULL
};

// Инициализация модуля клавиатуры
void keyboard_init(void) {
    // Очищаем состояние
    kbd_state.modifiers = 0;
    kbd_state.extended_key = false;
    kbd_state.key_released = false;
    kbd_state.led_state = 0;
    kbd_state.last_char = 0;
    kbd_state.last_scancode = 0;
    
    // Инициализируем названия для расширенных клавиш
    for (int i = 89; i < 256; i++) {
        if (!scancode_names[i]) {
            scancode_names[i] = "UNKNOWN";
        }
    }
    
    // Настраиваем клавиатуру
    keyboard_set_leds(0); // Все светодиоды выключены
}

// Основной обработчик прерываний клавиатуры
void keyboard_handler(void) {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    
    // Всегда отправляем EOI сразу после чтения данных
    outb(0x20, 0x20);
    
    keyboard_event_t event = {0};
    
    // Обработка расширенных клавиш (префикс E0)
    if (scancode == 0xE0) {
        kbd_state.extended_key = true;
        return; // Ждем следующий байт
    }
    
    // Определяем, нажата клавиша или отпущена
    bool key_pressed = !(scancode & 0x80);
    if (!key_pressed) {
        scancode &= 0x7F; // Убираем бит отпускания
    }
    
    // Формируем полный сканкод для расширенных клавиш
    uint16_t full_scancode = kbd_state.extended_key ? (0xE000 | scancode) : scancode;
    
    // Обновляем модификаторы
    update_modifiers(scancode, key_pressed, kbd_state.extended_key);
    
    // Заполняем структуру события
    event.scancode = full_scancode;
    event.modifiers = kbd_state.modifiers;
    event.pressed = key_pressed;
    event.extended = kbd_state.extended_key;
    
    // Для расширенных клавиш (стрелки, Delete и т.д.) не возвращаем ASCII символ
    if (kbd_state.extended_key) {
        event.character = 0; // Эти клавиши не имеют ASCII представления
    } else {
        event.character = keyboard_scancode_to_char(scancode, kbd_state.modifiers);
    }
    
    // Сохраняем в состоянии
    kbd_state.last_scancode = full_scancode;
    kbd_state.last_char = event.character;
    
    // Вызываем callback если установлен - только для печатных символов или важных клавиш
    if (kbd_callback && key_pressed) {
        // Обрабатываем только ASCII символы или специальные клавиши как Enter, Backspace
        if (event.character != 0 || scancode == KEY_ENTER || scancode == KEY_BACKSPACE) {
            kbd_callback(&event);
        }
    }
    
    // Сбрасываем флаг расширенной клавиши
    kbd_state.extended_key = false;
}

// Обновление состояния модификаторов
static void update_modifiers(uint8_t scancode, bool pressed, bool extended) {
    uint8_t modifier_bit = 0;
    
    switch (scancode) {
        case KEY_LSHIFT:
            modifier_bit = MODIFIER_LSHIFT;
            break;
        case KEY_RSHIFT:
            modifier_bit = MODIFIER_RSHIFT;
            break;
        case KEY_LCTRL:
            if (!extended) modifier_bit = MODIFIER_LCTRL;
            break;
        case KEY_LALT:
            if (!extended) modifier_bit = MODIFIER_LALT;
            break;
        case KEY_CAPSLOCK:
            if (pressed) {
                kbd_state.modifiers ^= MODIFIER_CAPSLOCK;
                keyboard_set_leds(kbd_state.led_state ^ LED_CAPSLOCK);
            }
            return;
        case KEY_NUMLOCK:
            if (pressed) {
                kbd_state.modifiers ^= MODIFIER_NUMLOCK;
                keyboard_set_leds(kbd_state.led_state ^ LED_NUMLOCK);
            }
            return;
    }
    
    // Обработка расширенных модификаторов
    if (extended) {
        switch (scancode) {
            case KEY_LCTRL: // RCTRL имеет тот же сканкод, но с префиксом E0
                modifier_bit = MODIFIER_RCTRL;
                break;
            case KEY_LALT:  // RALT имеет тот же сканкод, но с префиксом E0
                modifier_bit = MODIFIER_RALT;
                break;
        }
    }
    
    // Обновляем состояние модификатора
    if (modifier_bit) {
        if (pressed) {
            kbd_state.modifiers |= modifier_bit;
        } else {
            kbd_state.modifiers &= ~modifier_bit;
        }
    }
}

// Преобразование сканкода в ASCII символ
char keyboard_scancode_to_char(uint8_t scancode, uint8_t modifiers) {
    // scancode уже uint8_t, поэтому всегда < 256
    
    char c = 0;
    bool shift_active = IS_SHIFT_PRESSED(modifiers);
    bool caps_active = IS_CAPS_ACTIVE(modifiers);
    
    // Выбираем таблицу в зависимости от Shift
    if (shift_active) {
        c = scancode_ascii_shift[scancode];
    } else {
        c = scancode_ascii_normal[scancode];
    }
    
    // Обрабатываем Caps Lock для букв
    if (caps_active && c >= 'a' && c <= 'z') {
        c = c - 'a' + 'A';
    } else if (caps_active && c >= 'A' && c <= 'Z' && !shift_active) {
        c = c - 'A' + 'a';
    }
    
    return c;
}

// Получение названия клавиши по сканкоду
const char* keyboard_scancode_to_name(uint16_t scancode) {
    if (scancode & 0xE000) {
        // Расширенная клавиша
        uint8_t base_code = scancode & 0xFF;
        switch (base_code) {
            case 0x1C: return "KP_ENTER";
            case 0x1D: return "RCTRL";
            case 0x35: return "KP_/";
            case 0x38: return "RALT";
            case 0x47: return "HOME";
            case 0x48: return "UP";
            case 0x49: return "PAGEUP";
            case 0x4B: return "LEFT";
            case 0x4D: return "RIGHT";
            case 0x4F: return "END";
            case 0x50: return "DOWN";
            case 0x51: return "PAGEDOWN";
            case 0x52: return "INSERT";
            case 0x53: return "DELETE";
            default: return "E0_UNKNOWN";
        }
    } else {
        uint8_t code = scancode & 0xFF;
        if (scancode_names[code]) {
            return scancode_names[code];
        }
    }
    return "UNKNOWN";
}

// Установка callback функции
void keyboard_set_callback(keyboard_callback_t callback) {
    kbd_callback = callback;
}

// Получение текущих модификаторов
uint8_t keyboard_get_modifiers(void) {
    return kbd_state.modifiers;
}

// Проверка нажатия конкретной клавиши (упрощенная версия)
bool keyboard_is_key_pressed(uint16_t scancode) {
    // В полной реализации здесь был бы массив состояний всех клавиш
    return kbd_state.last_scancode == scancode;
}

// Установка состояния светодиодов
void keyboard_set_leds(uint8_t led_state) {
    kbd_state.led_state = led_state;
    
    // Отправляем команду установки LED
    outb(KEYBOARD_DATA_PORT, KBD_CMD_SET_LEDS);
    // Ждем ACK (в реальной системе нужна проверка)
    outb(KEYBOARD_DATA_PORT, led_state);
}

// Получение указателя на состояние клавиатуры
keyboard_state_t* keyboard_get_state(void) {
    return &kbd_state;
}