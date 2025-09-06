#include "../../include/scheduler.h"
#include "../../include/memory.h"
#include "../../include/utils.h"

// Глобальные переменные планировщика
static task_t* current_task = NULL;
static task_t* task_list = NULL;
static uint32_t next_task_id = 1;

void init_scheduler(void) {
    current_task = NULL;
    task_list = NULL;
    next_task_id = 1;
}

task_t* create_task(const char* name, void (*entry_point)(void), uint32_t priority) {
    task_t* task = (task_t*)kmalloc(sizeof(task_t));
    if (!task) return NULL;
    
    task->stack = (uint32_t*)kmalloc(TASK_STACK_SIZE);
    if (!task->stack) {
        kfree(task);
        return NULL;
    }
    
    task->id = next_task_id++;
    strcpy(task->name, name);
    task->state = TASK_STATE_READY;
    task->priority = priority;
    task->stack_size = TASK_STACK_SIZE;
    task->time_slice = 10;
    task->elf_loader = NULL;
    task->next = NULL;
    
    memset(&task->regs, 0, sizeof(registers_t));
    task->regs.eip = (uint32_t)entry_point;
    task->regs.esp = (uint32_t)task->stack + TASK_STACK_SIZE - 4;
    task->regs.cs = 0x08;
    task->regs.ds = 0x10;
    task->regs.es = 0x10;
    task->regs.fs = 0x10;
    task->regs.gs = 0x10;
    task->regs.ss = 0x10;
    task->regs.eflags = 0x202;
    
    if (task_list == NULL) {
        task_list = task;
        current_task = task;
    } else {
        task_t* last = task_list;
        while (last->next) {
            last = last->next;
        }
        last->next = task;
    }
    
    return task;
}

void schedule(void) {
    // Simplified implementation
}

void task_yield(void) {
    schedule();
}

void switch_to_task(task_t* task) {
    (void)task;
    // Simplified implementation
}

task_t* get_current_task(void) {
    return current_task;
}