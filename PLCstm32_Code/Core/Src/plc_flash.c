/*
 * plc_flash.c
 *
 *  Created on: 12 abr 2026
 *      Author: Yo
 */

#include <string.h>

#include "stm32c0xx_hal.h"

#include "plc_flash.h"
#include "plc_config.h"



__attribute__((section(".user_flash"), used))
const uint8_t user_program[] =
	{
		U16(3),

		OP_AND, CFG_NONE,
		U16(SIG_I(0)),
		U16(SIG_I(1)),
		U16(SIG_NONE),
		U16(SIG_NONE),
		U16(SIG_Q(0)),
		U16(0),
		U16(0),

		OP_TON, CFG_TIME_1S,
		U16(SIG_B(0)),
		U16(SIG_NONE),
		U16(SIG_NONE),
		U16(SIG_NONE),
		U16(SIG_Q(1)),
		U16(5),
		U16(0),

		OP_T_RANDOM, CFG_TIME_10MS,
		U16(SIG_I(2)),
		U16(SIG_NONE),
		U16(SIG_NONE),
		U16(SIG_NONE),
		U16(SIG_Q(2)),
		U16(500),
		U16(200)

	};



void flash_erase_user_area(void);
void flash_write_user(uint32_t addr, uint8_t *data, uint32_t size);
void flash_read_user(uint32_t address, uint8_t *data, uint32_t size);


void flash_erase_user_area(void)
{
	HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef erase;
    uint32_t page_error;

    erase.TypeErase = FLASH_TYPEERASE_PAGES;
    erase.Page = (USER_FLASH_START - FLASH_BASE) / FLASH_PAGE_SIZE;
    erase.NbPages = USER_FLASH_PAGES;

    HAL_FLASHEx_Erase(&erase, &page_error);

    HAL_FLASH_Lock();
}


void flash_write_user(uint32_t address, uint8_t *data, uint32_t size)
{
    HAL_FLASH_Unlock();

    for (uint32_t i = 0; i < size; i += 8)
    {
    	uint64_t val = 0;

        for (int j = 0; j < 8; j++)
        {
            val |= ((uint64_t)data[i + j]) << (8 * j);
        }

        HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address + i, val);
    }

    HAL_FLASH_Lock();
}



void flash_read_user(uint32_t address, uint8_t *data, uint32_t size)
{
	 memcpy(data, (uint8_t*)address, size);

}



/*
uint32_t crc_rx = *(uint32_t*)&rx_buffer[len - 4];
uint32_t crc_ok = crc32_calc(rx_buffer, len - 4);

if (crc_rx == crc_ok)
{
    flash_write_page(rx_buffer);
}
*/


