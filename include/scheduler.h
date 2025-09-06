#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "types.h"
#include "elf.h"

// Планировщик задач
#define MAX_TASKS           8           // Максимум задач
#define TASK_STACK_SIZE     4096        // Размер стека для каждой задачи

#define TASK_STATE_RUNNING  0           // Выполняется
#define TASK_STATE_READY    1           // Готова к выполнению
#define TASK_STATE_BLOCKED  2           // Заблокирована
#define TASK_STATE_DEAD     3           // Завершена

// Состояние регистров для переключения контекста
typedef struct {
    uint32_t eax, ebx, ecx, edx;
    uint32_t esi, edi, esp, ebp;
    uint32_t eip, eflags;
    uint16_t cs, ds, es, fs, gs, ss;
} registers_t;

// Структура задачи
typedef struct task {
    uint32_t id;                         // ID задачи
    char name[32];                       // Имя задачи
    uint32_t state;                      // Состояние задачи
    uint32_t priority;                   // Приоритет (0-высший, 255-низший)
    registers_t regs;                    // Сохраненные регистры
    uint32_t* stack;                     // Стек задачи
    uint32_t stack_size;                 // Размер стека
    uint32_t time_slice;                 // Оставшееся время выполнения
    elf_loader_t* elf_loader;            // ELF-загрузчик для этой задачи
    struct task* next;                   // Следующая задача в списке
} task_t;

// Функции планировщика
void init_scheduler(void);
task_t* create_task(const char* name, void (*entry_point)(void), uint32_t priority);
void schedule(void);
void task_yield(void);
void switch_to_task(task_t* task);
task_t* get_current_task(void);

#endif // SCHEDULER_H