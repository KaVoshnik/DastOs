// kernel.c
void terminal_putchar(char c) {
    char* video = (char*)0xB8000;
    static int pos = 0;
    video[pos++] = c;
    video[pos++] = 0x07;
}

void terminal_writestring(const char* data) {
    for (int i = 0; data[i] != '\0'; i++)
        terminal_putchar(data[i]);
}

void kernel_main() {
    terminal_writestring("Welcome to MyOS!\n");
}