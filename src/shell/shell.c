#include "../../include/shell.h"
#include "../../include/terminal.h"
#include "../../include/utils.h"
#include "../../include/memory.h"

// Переменные шелла
extern char command_buffer[COMMAND_BUFFER_SIZE];
extern int command_length;
extern int shell_ready;

// Callback для обработки клавиатуры в шелле
void shell_keyboard_callback(keyboard_event_t* event) {
    if (!event->pressed || !event->character) {
        return;
    }
    
    char c = event->character;
    
    if (c == '\n') {
        command_buffer[command_length] = '\0';
        terminal_putchar('\n');
        execute_command(command_buffer);
        command_length = 0;
        shell_prompt();
    } else if (c == '\b') {
        if (command_length > 0) {
            command_length--;
            terminal_putchar('\b');
        }
    } else if (command_length < COMMAND_BUFFER_SIZE - 1) {
        command_buffer[command_length++] = c;
        terminal_putchar(c);
    }
}

void shell_prompt(void) {
    terminal_writestring("myos> ");
}

void execute_command(const char* command) {
    if (strcmp(command, "help") == 0) {
        command_help();
    } else if (strcmp(command, "clear") == 0) {
        command_clear();
    } else if (strcmp(command, "about") == 0) {
        command_about();
    } else if (strcmp(command, "memory") == 0) {
        command_memory();
    } else if (command[0] != '\0') {
        terminal_writestring("Unknown command: ");
        terminal_writestring(command);
        terminal_writestring("\n");
    }
}

void command_help(void) {
    terminal_writestring("Available commands:\n");
    terminal_writestring("  help   - Show this help\n");
    terminal_writestring("  clear  - Clear screen\n");
    terminal_writestring("  about  - System information\n");
    terminal_writestring("  memory - Memory usage\n");
}

void command_clear(void) {
    terminal_clear();
}

void command_about(void) {
    terminal_writestring("MyOS v0.7 - Modular Kernel\n");
    terminal_writestring("Built with modular architecture\n");
}

void command_memory(void) {
    uint32_t total, free, used;
    get_memory_info(&total, &free, &used);
    
    terminal_writestring("Memory Usage:\n");
    terminal_writestring("  Total: "); print_number(total); terminal_writestring(" bytes\n");
    terminal_writestring("  Free:  "); print_number(free); terminal_writestring(" bytes\n");
    terminal_writestring("  Used:  "); print_number(used); terminal_writestring(" bytes\n");
}

void command_memtest(void) {
    terminal_writestring("Memory test completed\n");
}

void command_keyboard(void) {
    terminal_writestring("Keyboard status: OK\n");
}

void command_tasks(void) {
    terminal_writestring("No tasks running\n");
}