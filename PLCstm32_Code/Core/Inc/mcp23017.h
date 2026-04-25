/*
 * mcp23017.h
 *
 *  Created on: 28 mar 2026
 *      Author: Yo
 */

#ifndef INC_MCP23017_H_
#define INC_MCP23017_H_

#include <stdint.h>

/* =========================================================
			Definición para MCP23017
========================================================= */

#define MCP23017_ADDR	 0x20 		// Dirección MCP23017 A0-A2 = 0   0x20

#define MCP23017_IODIRA  0x00     		// Registros del MCP23017
#define MCP23017_IODIRB  0x01
#define MCP23017_GPPUB   0x0D
#define MCP23017_GPIOA   0x12
#define MCP23017_GPIOB   0x13

/* =========================================================
			Funciones del MCP23017
========================================================= */

void MCP23017_Init(void);

int MCP23017_WritePortA(uint8_t value);
int MCP23017_ReadPortB(uint8_t *value);
uint8_t  MCP23017_Check(void);

#endif /* INC_MCP23017_H_ */
