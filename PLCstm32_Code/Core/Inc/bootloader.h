/*
 * bootloader.h
 *
 *  Created on: 19 abr 2026
 *      Author: Yo
 */

#ifndef INC_BOOTLOADER_H_
#define INC_BOOTLOADER_H_


#include <stdint.h>

#define BOOT_MAGIC    0xB007C0DE

#define BOOT_NORMAL   0
#define BOOT_REQUEST  1
#define BOOT_UPDATE   2

typedef struct
{
    uint32_t magic;
    uint32_t flag;
    uint32_t crc;
} boot_ctrl_t;


// variables externas
extern boot_ctrl_t boot_ctrl;

void boot_init(void);

void system_start(void);

void bootloader_run(void);
void plc_run(void);

void uart_process_byte(uint8_t byte);

uint32_t calc_boot_crc(boot_ctrl_t *b);

/* CRC32 */
uint32_t crc32_calc(uint8_t *data, uint32_t len);


#endif /* INC_BOOTLOADER_H_ */
