#ifndef SHELL_H
#define SHELL_H

#include "types.h"
#include "keyboard.h"

// Шелл
#define COMMAND_BUFFER_SIZE 256

// Функции шелла
void shell_prompt(void);
void execute_command(const char* command);
void shell_keyboard_callback(keyboard_event_t* event);

// Команды шелла
void command_help(void);
void command_clear(void);
void command_about(void);
void command_memory(void);
void command_memtest(void);
void command_keyboard(void);
void command_tasks(void);

#endif // SHELL_H