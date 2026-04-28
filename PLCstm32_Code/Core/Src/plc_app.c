/*
 * plc_app.c
 *
 *  Created on: 15 abr 2026
 *      Author: Yo
 */

#include <string.h>


#include "plc_app.h"
#include "plc_nucleo.h"
#include "plc_flash.h"

static uint32_t x;


/* =========================================================
 			Para inicializar el PLC (Variables)
 ========================================================= */

void plc_init(void)
{
     memset(state, 0, sizeof(state));
     memset(flags, 0, sizeof(flags));
     memset(B, 0, sizeof(B));
}

/* =========================================================

========================================================= */

uint32_t get_uid(void)
{
    uint32_t uid = 0;

    uid ^= *(uint32_t*)0x1FFF0C00;
    uid ^= *(uint32_t*)0x1FFF0C04;
    uid ^= *(uint32_t*)0x1FFF0C08;

    return uid;
}

/* =========================================================

========================================================= */

void rng_init(void)
{
    uint32_t seed = 0;

    seed ^= get_uid();                 // único por chip
    seed ^= GetTick();             // tiempo arranque
    seed ^= (GetTick() << 16);     // variación extra

    x = seed; // inicializa xorshift
}

/* =========================================================

========================================================= */

uint32_t better_rand(void)
{
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;

	// remezcla genera con UID
	x ^= get_uid();

	return x;
}

/* =========================================================
			SCAN
========================================================= */

void plc_scan(void)
{
    uint8_t *p = (uint8_t*)USER_FLASH_START;

    // leer número de bloques (2 bytes little endian)
    uint16_t num_blocks = (uint16_t)(p[0] | (p[1] << 8));
    p += 2;

    for (uint16_t i = 0; i < num_blocks; i++)
    {
    	Block16 *blk = (Block16*)p;

    	execute_block(blk, i);

    	p += sizeof(Block16);
    }
}


/* =========================================================
			Para detectar pulsacion pulsador
========================================================= */

uint8_t detectar_pulsacion(void)
{
	 static uint8_t last = 0;

	 uint8_t current = ((GPIOA->IDR & GPIO_IDR_ID8) == 0);
	 uint8_t edge = (current == 1 && last == 0);
	 last = current;
	 return edge;
}

/* =========================================================
			Para actualizar modo de funcionamiento
========================================================= */

void actualizar_modo(void)
{
    if (detectar_pulsacion())
    {
        if (mode == MODE_STOP)
            mode = MODE_RUN;
        else if (mode == MODE_RUN)
            mode = MODE_CARGA;
        else
            mode = MODE_STOP;
    }
}

/* =========================================================
			Parpadear el led segun modo funcionamiento
========================================================= */

void parpadeo_led(void)
{
    static uint32_t last = 0;
    uint32_t now = GetTick();

    uint32_t period = 0;

    switch (mode)
    {
        case MODE_STOP:  period = 1000; break;
        case MODE_RUN:   period =  300; break;
        case MODE_CARGA: period =  100; break;
        case MODE_ERROR: period =   25; break;
        default:         period = 2000; break;
    }

    if (now - last >= period)
    {
        last = now;

        GPIOB->ODR ^= (1U << 6); // toggle directo del bit LED
    }
}

/* =========================================================
				CRC Funciones
========================================================= */

uint32_t crc32_update(uint32_t crc, uint8_t data)
{
    crc ^= data;

    for (int i = 0; i < 8; i++)
    {
        if (crc & 1)
            crc = (crc >> 1) ^ 0xEDB88320;
        else
            crc >>= 1;
    }

    return crc;
}

uint32_t crc32_calc(uint8_t *data, uint32_t len)
{
    uint32_t crc = 0xFFFFFFFF;

    for (uint32_t i = 0; i < len; i++)
    {
        crc = crc32_update(crc, data[i]);
    }

    return ~crc;
}


uint32_t GetTick(void)
{
    return msTicks;
}


void Delay_ms(uint32_t ms)
{
    uint32_t start = msTicks;
    while ((msTicks - start) < ms);
}








