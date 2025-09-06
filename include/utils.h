#ifndef UTILS_H
#define UTILS_H

#include "types.h"

// Функции для работы с портами
static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Функции для работы с памятью
void* memset(void* dest, int val, size_t len);
void* memcpy(void* dest, const void* src, size_t len);

// Функции для работы со строками
int strlen(const char* str);
int strcmp(const char* str1, const char* str2);
void strcpy(char* dest, const char* src);
int strncmp(const char* str1, const char* str2, size_t n);
char* strncpy(char* dest, const char* src, size_t n);

#endif // UTILS_H