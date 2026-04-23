/*
 * plc_config.c
 *
 *  Created on: 29 mar 2026
 *      Author: Yo
 */


#include "plc_config.h"


/* =========================================================
			Definición
========================================================= */

volatile uint8_t system_flags = 0;

volatile uint32_t I = 0;
volatile uint32_t Q = 0;
volatile uint64_t M = 0;

volatile SystemMode_t mode = MODE_STOP;
