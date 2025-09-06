#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

// Управление памятью
#define HEAP_START 0x400000    // Начало кучи (4 MB)
#define HEAP_SIZE  0x100000    // Размер кучи (1 MB)

// Структура блока памяти
typedef struct memory_block {
    size_t size;                    // Размер блока
    int is_free;                    // Флаг: свободен ли блок
    struct memory_block* next;      // Указатель на следующий блок
    struct memory_block* prev;      // Указатель на предыдущий блок
} memory_block_t;

// Функции управления памятью
void init_memory_management(void);
void* kmalloc(size_t size);
void kfree(void* ptr);
void get_memory_info(uint32_t* total, uint32_t* free, uint32_t* used);

#endif // MEMORY_H