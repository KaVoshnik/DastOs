#ifndef TIMER_H
#define TIMER_H

#include "types.h"

// Таймер (PIT - Programmable Interval Timer)
#define PIT_FREQUENCY   1193182
#define TIMER_FREQUENCY 100          // 100 Hz = 10ms тики
#define PIT_COMMAND     0x43
#define PIT_DATA0       0x40

// Глобальные переменные таймера
extern uint32_t timer_ticks;
extern uint32_t timer_frequency;

// Функции таймера
void init_timer(uint32_t frequency);

#endif // TIMER_H