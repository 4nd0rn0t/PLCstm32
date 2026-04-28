/*
 * bootloader.c
 *
 *  Created on: 19 abr 2026
 *      Author: Yo
 */

#include "bootloader.h"

#include "stm32c0xx.h"				// Codigo tipo CMSIS (nivel bajo)

#include "config.h"

#include <string.h>

void bootloader_run(void);
void plc_run(void);


__attribute__((section(".noinit")))
boot_ctrl_t boot_ctrl;

uint32_t calc_boot_crc(boot_ctrl_t *b)
{
    return crc32_calc((uint8_t*)&b->magic, sizeof(uint32_t) * 2); // magic + flag
}


static void validate_boot_ctrl(void)
{

    if (boot_ctrl.magic != BOOT_MAGIC)
    {
        boot_ctrl.magic = BOOT_MAGIC;
        boot_ctrl.flag  = BOOT_NORMAL;
    }

    if (boot_ctrl.crc != calc_boot_crc(&boot_ctrl))
    {
        boot_ctrl.flag = BOOT_NORMAL;
    }
    	boot_ctrl.crc = calc_boot_crc(&boot_ctrl);
}




void system_start(void)
{
    validate_boot_ctrl();

    if (boot_ctrl.flag == BOOT_REQUEST ||
        boot_ctrl.flag == BOOT_UPDATE)
    {
        if (boot_ctrl.flag == BOOT_REQUEST)
        {
            boot_ctrl.flag = BOOT_UPDATE;
            boot_ctrl.crc  = calc_boot_crc(&boot_ctrl);
        }

        bootloader_run();
    }
    else
    {
        plc_run();
    }
}



void bootloader_run(void)
{
    // Encender LED
    //HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
	GPIOB->BSRR = (1U << (6 + 16)); // reset   encender LED

    uint32_t start = GetTick();

    while (1)
    {
        // si pasan 10 segundos
         if ((GetTick() - start) >= 10000)
        {




        	  // Poner en modo PLC, cuando termine la carga y hacer reset
            boot_ctrl.flag = BOOT_NORMAL;
            boot_ctrl.crc  = calc_boot_crc(&boot_ctrl);

            NVIC_SystemReset();
        }

        // aquí parsing UART + flash
    }
}


void uart_process_byte(uint8_t byte)
{
    static char buffer[16];
    static uint8_t index = 0;

    // fin de línea
    if (byte == '\n' || byte == '\r')
    {
        buffer[index] = 0;

        // comprobar comando poner mas caracters o valor hex
        if (strcmp(buffer, "CARGA") == 0)
        {

        	boot_ctrl.magic = BOOT_MAGIC;
        	boot_ctrl.flag  = BOOT_REQUEST;

        	boot_ctrl.crc  = calc_boot_crc(&boot_ctrl);

        	NVIC_SystemReset();
        }

        index = 0;		// reset buffer
    }
    else
    {
        // Guardar carácter si hay espacio buffer
        if (index < sizeof(buffer) - 1)
        {
            buffer[index++] = byte;
        }
        else
        {
            // overflow → reset
            index = 0;
        }
    }
}


