#ifndef ATA_H
#define ATA_H

#include "types.h"

// ATA Порты для Primary и Secondary контроллеров
#define ATA_PRIMARY_BASE        0x1F0
#define ATA_PRIMARY_CTRL        0x3F6
#define ATA_SECONDARY_BASE      0x170
#define ATA_SECONDARY_CTRL      0x376

// Регистры ATA (смещения от базового адреса)
#define ATA_REG_DATA            0x00
#define ATA_REG_ERROR           0x01
#define ATA_REG_FEATURES        0x01
#define ATA_REG_SECCOUNT        0x02
#define ATA_REG_LBA_LOW         0x03
#define ATA_REG_LBA_MID         0x04
#define ATA_REG_LBA_HIGH        0x05
#define ATA_REG_DRIVE           0x06
#define ATA_REG_STATUS          0x07
#define ATA_REG_COMMAND         0x07

// Биты статуса ATA
#define ATA_STATUS_BSY          0x80    // Busy
#define ATA_STATUS_DRDY         0x40    // Drive Ready
#define ATA_STATUS_DF           0x20    // Drive Fault
#define ATA_STATUS_DSC          0x10    // Drive Seek Complete
#define ATA_STATUS_DRQ          0x08    // Data Request
#define ATA_STATUS_CORR         0x04    // Corrected data
#define ATA_STATUS_IDX          0x02    // Index
#define ATA_STATUS_ERR          0x01    // Error

// Команды ATA
#define ATA_CMD_READ_SECTORS    0x20
#define ATA_CMD_WRITE_SECTORS   0x30
#define ATA_CMD_IDENTIFY        0xEC
#define ATA_CMD_FLUSH           0xE7

// Биты регистра Drive
#define ATA_DRIVE_MASTER        0xA0
#define ATA_DRIVE_SLAVE         0xB0
#define ATA_DRIVE_LBA           0x40

// Размеры
#define ATA_SECTOR_SIZE         512
#define ATA_IDENTIFY_SIZE       256

// Структура информации о диске
typedef struct {
    uint16_t base_port;         // Базовый порт (0x1F0 или 0x170)
    uint16_t ctrl_port;         // Порт управления (0x3F6 или 0x376)
    uint8_t drive_num;          // Номер диска (0 - Master, 1 - Slave)
    uint8_t drive_select;       // Значение для регистра Drive
    uint32_t sectors;           // Количество секторов на диске
    uint16_t cylinders;         // Количество цилиндров
    uint16_t heads;             // Количество головок
    uint16_t sectors_per_track; // Секторов на дорожку
    char model[41];             // Модель диска (40 символов + \0)
    bool present;               // Диск присутствует
    bool lba_supported;         // Поддержка LBA адресации
} ata_device_t;

// Глобальные переменные
extern ata_device_t ata_devices[4]; // Primary Master/Slave, Secondary Master/Slave
extern int ata_device_count;

// Функции ATA драйвера
void ata_init(void);
bool ata_identify(ata_device_t* device);
bool ata_read_sectors(ata_device_t* device, uint32_t lba, uint8_t sector_count, uint8_t* buffer);
bool ata_write_sectors(ata_device_t* device, uint32_t lba, uint8_t sector_count, uint8_t* buffer);
void ata_wait_ready(ata_device_t* device);
void ata_wait_drq(ata_device_t* device);
uint8_t ata_read_status(ata_device_t* device);
void ata_select_drive(ata_device_t* device);

// Вспомогательные функции
ata_device_t* ata_get_primary_master(void);
ata_device_t* ata_get_device(int index);
void ata_list_devices(void);

#endif // ATA_H