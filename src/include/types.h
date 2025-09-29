#ifndef TYPES_H
#define TYPES_H

// Basic type definitions
typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;
typedef unsigned long long uint64_t;

typedef signed char    int8_t;
typedef signed short   int16_t;
typedef signed int     int32_t;
typedef signed long long int64_t;

typedef unsigned int   size_t;
typedef signed int     ssize_t;

// Pointer types
typedef uint32_t       uintptr_t;
typedef int32_t        intptr_t;

// Boolean type
#ifndef __cplusplus
#ifndef bool
typedef enum {
    FALSE = 0,
    TRUE = 1
} bool_t;
#define bool bool_t
#define false FALSE
#define true TRUE
#endif
#endif

// NULL definition
#ifndef NULL
#define NULL ((void*)0)
#endif

// Min/Max values
#define UINT8_MAX   255
#define UINT16_MAX  65535
#define UINT32_MAX  4294967295U

#define INT8_MIN    (-128)
#define INT8_MAX    127
#define INT16_MIN   (-32768)
#define INT16_MAX   32767
#define INT32_MIN   (-2147483647 - 1)
#define INT32_MAX   2147483647

#endif // TYPES_H