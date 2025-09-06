#include "../../include/memory.h"
#include "../../include/utils.h"

// Глобальные переменные управления памятью
memory_block_t* heap_start = NULL;
uint32_t total_memory = 0;
uint32_t free_memory = 0;
uint32_t used_memory = 0;

// Инициализация системы управления памятью
void init_memory_management() {
    heap_start = (memory_block_t*)HEAP_START;
    heap_start->size = HEAP_SIZE - sizeof(memory_block_t);
    heap_start->is_free = 1;
    heap_start->next = NULL;
    heap_start->prev = NULL;
    
    total_memory = HEAP_SIZE;
    free_memory = HEAP_SIZE - sizeof(memory_block_t);
    used_memory = sizeof(memory_block_t);
}

// Выделение памяти
void* kmalloc(size_t size) {
    if (size == 0) return NULL;
    
    size = (size + 3) & ~3; // Выравнивание по 4 байта
    
    memory_block_t* current = heap_start;
    
    while (current != NULL) {
        if (current->is_free && current->size >= size) {
            if (current->size > size + sizeof(memory_block_t) + 16) {
                memory_block_t* new_block = (memory_block_t*)((uint32_t)current + sizeof(memory_block_t) + size);
                new_block->size = current->size - size - sizeof(memory_block_t);
                new_block->is_free = 1;
                new_block->next = current->next;
                new_block->prev = current;
                
                if (current->next) {
                    current->next->prev = new_block;
                }
                current->next = new_block;
                current->size = size;
            }
            
            current->is_free = 0;
            free_memory -= current->size;
            used_memory += current->size;
            
            return (void*)((uint32_t)current + sizeof(memory_block_t));
        }
        current = current->next;
    }
    
    return NULL;
}

// Освобождение памяти
void kfree(void* ptr) {
    if (ptr == NULL) return;
    
    memory_block_t* block = (memory_block_t*)((uint32_t)ptr - sizeof(memory_block_t));
    
    if (block->is_free) return;
    
    block->is_free = 1;
    free_memory += block->size;
    used_memory -= block->size;
    
    // Объединение соседних свободных блоков
    if (block->next && block->next->is_free) {
        block->size += block->next->size + sizeof(memory_block_t);
        if (block->next->next) {
            block->next->next->prev = block;
        }
        block->next = block->next->next;
    }
    
    if (block->prev && block->prev->is_free) {
        block->prev->size += block->size + sizeof(memory_block_t);
        if (block->next) {
            block->next->prev = block->prev;
        }
        block->prev->next = block->next;
    }
}

// Получение информации о памяти
void get_memory_info(uint32_t* total, uint32_t* free, uint32_t* used) {
    *total = total_memory;
    *free = free_memory;
    *used = used_memory;
}