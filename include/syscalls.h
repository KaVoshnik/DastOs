#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "types.h"

// Системные вызовы
#define SYS_EXIT    0
#define SYS_WRITE   1
#define SYS_READ    2
#define SYS_OPEN    3
#define SYS_CLOSE   4
#define SYS_FORK    5
#define SYS_EXEC    6
#define SYS_WAIT    7
#define SYS_GETPID  8
#define SYS_YIELD   9
#define SYS_SLEEP   10

// Функция обработки системных вызовов
int handle_syscall(int syscall_num, int arg0, int arg1, int arg2, int arg3, int arg4);

// Внешние функции из ASM
extern int syscall0(int syscall_num);
extern int syscall1(int syscall_num, int arg0);
extern int syscall2(int syscall_num, int arg0, int arg1);
extern int syscall3(int syscall_num, int arg0, int arg1, int arg2);
extern int syscall4(int syscall_num, int arg0, int arg1, int arg2, int arg3);
extern int syscall5(int syscall_num, int arg0, int arg1, int arg2, int arg3, int arg4);

#endif // SYSCALLS_H