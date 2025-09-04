#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.h"

// Коды портов клавиатуры
#define KEYBOARD_DATA_PORT    0x60
#define KEYBOARD_STATUS_PORT  0x64
#define KEYBOARD_COMMAND_PORT 0x64

// Команды клавиатуры
#define KBD_CMD_RESET         0xFF
#define KBD_CMD_ENABLE        0xF4
#define KBD_CMD_DISABLE       0xF5
#define KBD_CMD_SET_LEDS      0xED
#define KBD_CMD_SET_SCANCODE  0xF0

// Коды клавиш (Set 1 Scancode)
#define KEY_ESC               0x01
#define KEY_1                 0x02
#define KEY_2                 0x03
#define KEY_3                 0x04
#define KEY_4                 0x05
#define KEY_5                 0x06
#define KEY_6                 0x07
#define KEY_7                 0x08
#define KEY_8                 0x09
#define KEY_9                 0x0A
#define KEY_0                 0x0B
#define KEY_MINUS             0x0C
#define KEY_EQUALS            0x0D
#define KEY_BACKSPACE         0x0E
#define KEY_TAB               0x0F
#define KEY_Q                 0x10
#define KEY_W                 0x11
#define KEY_E                 0x12
#define KEY_R                 0x13
#define KEY_T                 0x14
#define KEY_Y                 0x15
#define KEY_U                 0x16
#define KEY_I                 0x17
#define KEY_O                 0x18
#define KEY_P                 0x19
#define KEY_LBRACKET          0x1A
#define KEY_RBRACKET          0x1B
#define KEY_ENTER             0x1C
#define KEY_LCTRL             0x1D
#define KEY_A                 0x1E
#define KEY_S                 0x1F
#define KEY_D                 0x20
#define KEY_F                 0x21
#define KEY_G                 0x22
#define KEY_H                 0x23
#define KEY_J                 0x24
#define KEY_K                 0x25
#define KEY_L                 0x26
#define KEY_SEMICOLON         0x27
#define KEY_QUOTE             0x28
#define KEY_GRAVE             0x29
#define KEY_LSHIFT            0x2A
#define KEY_BACKSLASH         0x2B
#define KEY_Z                 0x2C
#define KEY_X                 0x2D
#define KEY_C                 0x2E
#define KEY_V                 0x2F
#define KEY_B                 0x30
#define KEY_N                 0x31
#define KEY_M                 0x32
#define KEY_COMMA             0x33
#define KEY_PERIOD            0x34
#define KEY_SLASH             0x35
#define KEY_RSHIFT            0x36
#define KEY_KPASTERISK        0x37
#define KEY_LALT              0x38
#define KEY_SPACE             0x39
#define KEY_CAPSLOCK          0x3A

// Функциональные клавиши
#define KEY_F1                0x3B
#define KEY_F2                0x3C
#define KEY_F3                0x3D
#define KEY_F4                0x3E
#define KEY_F5                0x3F
#define KEY_F6                0x40
#define KEY_F7                0x41
#define KEY_F8                0x42
#define KEY_F9                0x43
#define KEY_F10               0x44

// Цифровая клавиатура
#define KEY_NUMLOCK           0x45
#define KEY_SCROLLLOCK        0x46
#define KEY_KP7               0x47
#define KEY_KP8               0x48
#define KEY_KP9               0x49
#define KEY_KPMINUS           0x4A
#define KEY_KP4               0x4B
#define KEY_KP5               0x4C
#define KEY_KP6               0x4D
#define KEY_KPPLUS            0x4E
#define KEY_KP1               0x4F
#define KEY_KP2               0x50
#define KEY_KP3               0x51
#define KEY_KP0               0x52
#define KEY_KPDOT             0x53

// Расширенные клавиши (с префиксом E0)
#define KEY_F11               0x57
#define KEY_F12               0x58
#define KEY_KPENTER           0xE01C
#define KEY_RCTRL             0xE01D
#define KEY_KPSLASH           0xE035
#define KEY_RALT              0xE038
#define KEY_HOME              0xE047
#define KEY_UP                0xE048
#define KEY_PAGEUP            0xE049
#define KEY_LEFT              0xE04B
#define KEY_RIGHT             0xE04D
#define KEY_END               0xE04F
#define KEY_DOWN              0xE050
#define KEY_PAGEDOWN          0xE051
#define KEY_INSERT            0xE052
#define KEY_DELETE            0xE053

// Модификаторы клавиш
#define MODIFIER_LSHIFT       0x01
#define MODIFIER_RSHIFT       0x02
#define MODIFIER_LCTRL        0x04
#define MODIFIER_RCTRL        0x08
#define MODIFIER_LALT         0x10
#define MODIFIER_RALT         0x20
#define MODIFIER_CAPSLOCK     0x40
#define MODIFIER_NUMLOCK      0x80

// Структура состояния клавиатуры
typedef struct {
    uint8_t modifiers;          // Битовая маска модификаторов
    bool extended_key;          // Флаг расширенной клавиши (E0)
    bool key_released;          // Флаг отпускания клавиши
    uint8_t led_state;          // Состояние светодиодов
    char last_char;             // Последний введенный символ
    uint16_t last_scancode;     // Последний сканкод
} keyboard_state_t;

// Структура события клавиатуры
typedef struct {
    uint16_t scancode;          // Сканкод клавиши
    char character;             // ASCII символ (если применимо)
    uint8_t modifiers;          // Активные модификаторы
    bool pressed;               // true = нажата, false = отпущена
    bool extended;              // Расширенная клавиша
} keyboard_event_t;

// Указатель на обработчик событий клавиатуры
typedef void (*keyboard_callback_t)(keyboard_event_t* event);

// Функции модуля клавиатуры
void keyboard_init(void);
void keyboard_handler(void);
void keyboard_set_callback(keyboard_callback_t callback);
char keyboard_scancode_to_char(uint8_t scancode, uint8_t modifiers);
const char* keyboard_scancode_to_name(uint16_t scancode);
uint8_t keyboard_get_modifiers(void);
bool keyboard_is_key_pressed(uint16_t scancode);
void keyboard_set_leds(uint8_t led_state);
keyboard_state_t* keyboard_get_state(void);

// LED флаги
#define LED_SCROLLLOCK  0x01
#define LED_NUMLOCK     0x02
#define LED_CAPSLOCK    0x04

// Вспомогательные макросы
#define IS_SHIFT_PRESSED(mods)    ((mods) & (MODIFIER_LSHIFT | MODIFIER_RSHIFT))
#define IS_CTRL_PRESSED(mods)     ((mods) & (MODIFIER_LCTRL | MODIFIER_RCTRL))
#define IS_ALT_PRESSED(mods)      ((mods) & (MODIFIER_LALT | MODIFIER_RALT))
#define IS_CAPS_ACTIVE(mods)      ((mods) & MODIFIER_CAPSLOCK)

#endif // KEYBOARD_H