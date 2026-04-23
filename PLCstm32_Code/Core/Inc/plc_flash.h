/*
 * plc_flash.h
 *
 *  Created on: 12 abr 2026
 *      Author: Yo
 */

#ifndef INC_PLC_FLASH_H_
#define INC_PLC_FLASH_H_


#define USER_FLASH_START ((uint32_t)0x08006800U)
#define USER_FLASH_END    ((uint32_t)0x08008000U)

//#define FLASH_PAGE_SIZE   ((uint32_t)0x00000800U)   // ya definido en el stm32c0xx_hal_flash.h
#define USER_FLASH_PAGES  ((uint32_t)3U)


/*
 * Convierte un uint16_t en 2 bytes:
 * byte bajo (LSB), byte alto (MSB)
 */
#define U16(x)   ((uint8_t)((x) & 0xFF)), ((uint8_t)(((x) >> 8) & 0xFF))






#endif /* INC_PLC_FLASH_H_ */
