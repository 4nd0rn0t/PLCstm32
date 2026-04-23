/*
 * terminal.h
 *
 *  Created on: 29 mar 2026
 *      Author: Yo
 */

#ifndef INC_TERMINAL_H_
#define INC_TERMINAL_H_

#include <stdint.h>

#include "plc_config.h"

//void UART_Print(char *msg);
const char* MCP23017_ModeToString(SystemMode_t mode);
void MCP23017_PrintStatus(uint8_t inputs, uint8_t outputs, SystemMode_t mode);
void Terminal_Imprimir(uint8_t entradas, uint8_t salidas, SystemMode_t mode);


#endif /* INC_TERMINAL_H_ */
