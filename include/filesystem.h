#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "types.h"

// Файловая система
#define FS_MAX_FILES        64          // Максимум файлов
#define FS_MAX_FILENAME     32          // Максимум символов в имени файла
#define FS_MAX_FILESIZE     1024        // Максимум байт в файле
#define FS_BLOCK_SIZE       64          // Размер блока данных
#define FS_MAX_BLOCKS       256         // Максимум блоков данных

#define FS_INODE_FREE       0           // Свободный inode
#define FS_INODE_FILE       1           // Обычный файл
#define FS_INODE_DIR        2           // Директория (пока не используется)

// Суперблок файловой системы
typedef struct {
    uint32_t magic;              // Магическое число для проверки
    uint32_t total_inodes;       // Общее количество inodes
    uint32_t free_inodes;        // Свободные inodes
    uint32_t total_blocks;       // Общее количество блоков данных
    uint32_t free_blocks;        // Свободные блоки
    uint32_t block_size;         // Размер блока данных
} fs_superblock_t;

// Индексный узел (inode) файла
typedef struct {
    uint8_t type;                        // Тип: свободный, файл, директория
    char filename[FS_MAX_FILENAME];      // Имя файла
    uint32_t size;                       // Размер файла в байтах
    uint32_t blocks[16];                 // Прямые указатели на блоки данных
    uint32_t created_time;               // Время создания (упрощенно)
    uint32_t modified_time;              // Время модификации
} fs_inode_t;

// Глобальное состояние файловой системы
typedef struct {
    fs_superblock_t superblock;          // Суперблок
    fs_inode_t inodes[FS_MAX_FILES];     // Таблица inodes
    uint8_t* data_blocks;                // Указатель на область данных
    uint8_t inode_bitmap[FS_MAX_FILES];  // Битовая карта занятых inodes
    uint8_t block_bitmap[FS_MAX_BLOCKS]; // Битовая карта занятых блоков
    int initialized;                     // Флаг инициализации
} fs_state_t;

// Функции файловой системы
void init_filesystem(void);
int fs_create_file(const char* filename);
int fs_delete_file(const char* filename);
int fs_write_file(const char* filename, const char* data, uint32_t size);
int fs_read_file(const char* filename, char* buffer, uint32_t max_size);
void fs_list_files(void);
int fs_file_exists(const char* filename);
fs_inode_t* fs_find_inode(const char* filename);

#endif // FILESYSTEM_H