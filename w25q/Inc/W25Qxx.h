#ifndef CAM_W25QXX_H
#define CAM_W25QXX_H

#include <stdint.h>

/**
 * Return status codes of  the driver. There can be multiple error codes at once (via masking with OR).
 * E.g., the code 0b110 means that there is an SPI error together with an invalid address.
 */
typedef enum W25Q_Status_ {
    W25Q_OK = 0b0,
    W25Q_NOT_SUPPORTED = 0b10,
    W25Q_SPI_COMM_ERROR = 0b100,
    W25Q_INVALID_ADDRESS = 0b1000,
    W25Q_INVALID_SECTOR = 0b10000,
} W25Q_Status;

typedef void (*GPIO_Function)(uint32_t);

typedef W25Q_Status (*SPI_Function)(uint8_t *, uint16_t);

/**
 * Flash struct object. Holds a full description of the flash's organization such as the number of pages, sectors and blocks.
 * The values are calculated in W25Q_Init. Do not modify them.
 */
typedef struct W25Q_ {
    uint32_t flash_size_bytes;
    uint32_t pages;
    uint32_t page_size_byte;
    uint32_t sectors;
    uint32_t sector_size_byte;
    uint32_t pages_per_sector;
    uint32_t blocks_small;
    uint32_t block_small_size_byte;
    uint32_t pages_per_blocks_small;
    uint32_t blocks_large;
    uint32_t block_large_size_byte;
    uint32_t pages_per_blocks_large;
    GPIO_Function gpio_function;
    SPI_Function spi_write;
    SPI_Function spi_read;
} W25Q;

/**
 * The parameters describing the flash (from the data sheet). They depend on the flash model.
 * W25Q64FV, e.g., has 32768 pages, each with a size of 256 byte. 16 pages form a sector and either 256 or 128 pages
 * form a large or small block, respectively.
 */
typedef struct W25QInitParams_ {
    uint32_t pages;
    uint32_t page_size_byte;
    uint32_t pages_per_sector;
    uint32_t pages_per_blocks_small;
    uint32_t pages_per_blocks_large;
    GPIO_Function gpio_function;
    SPI_Function spi_write;
    SPI_Function spi_read;
} W25QInitParams;

/**
 * The JEDEC identifier of a flash (capacity, memory type and manufacturer).
 */
typedef union Q25QJEDECIdentifier_ {
    struct {
        uint8_t capacity;
        uint8_t memory_type;
        uint8_t manufacturer;
    };
    uint32_t jedec_identifier;
} W25QJEDECIdentifier;

/**
 * Initializes the flash with the given parameters. This does not build up a communication, yet. Instead, the flash's parameters
 * are calculated. Note that flashes with more blocks than W25Q_NUMER_OF_SMALL_BLOCKS_UNSUPPORTED are not supported.
 */
W25Q_Status W25Q_Init(W25Q *flash, const W25QInitParams *params);

/**
 * Sends a reset signal to the flash. Must be called before other operations.
 */
W25Q_Status W25Q_Reset(const W25Q *flash);

/**
 * Reads the JEDEC identifier of this flash, consisting of manufacturer, memory type and capacity.
 */
W25Q_Status W25Q_ReadJEDECIdentifier(const W25Q *flash, W25QJEDECIdentifier *out_id);

/**
 * Reads the data from the provided address and length into the buffer.
 */
W25Q_Status W25Q_Read(const W25Q *flash, uint32_t address, uint32_t size, uint8_t *buffer);

/**
 * Erases the given sector (smallest entity that can be deleted)
 */
W25Q_Status W25Q_EraseSector(const W25Q *flash, uint32_t sector);

/**
 * Generic write function. Writes size bytes from data at the given address.
 * The address can be arbitrary. If necessary, the affected section will be deleted. Note that section_buffer
 * must therefore be of the size sector_size_byte.
 */
W25Q_Status W25Q_Write(const W25Q *flash, uint32_t address, const uint8_t *data, uint32_t size,
                            uint8_t *sector_backup);

/**
 * Full chip erase. Long-running function, use with care.
 */
W25Q_Status W25Q_ChipErase(const W25Q *flash);

/**
 * Checks if the flash is currently busy.
 */
W25Q_Status W25Q_Busy(const W25Q *flash, uint32_t *is_busy);


#endif //CAM_W25QXX_H
