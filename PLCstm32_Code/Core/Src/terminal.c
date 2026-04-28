/*
 * terminal.c
 *
 *  Created on: 29 mar 2026
 *      Author: Yo
 */

#include "main.h"
#include "terminal.h"

static uint8_t ultima_I = 0xFF;
static uint8_t ultima_Q  = 0xFF;
static SystemMode_t ultimo_mode = MODE_UNKNOWN;

//extern volatile uint32_t cont_serial;


void UART_Print(const char *str)
{
    while (*str)
    {
        while (!(USART1->ISR & USART_ISR_TXE_TXFNF));
        USART1->TDR = *str++;
    }
}


void Terminal_Imprimir(uint8_t I, uint8_t Q, SystemMode_t mode)

{
	if (I != ultima_I || Q!= ultima_Q || mode != ultimo_mode)
	{
		MCP23017_PrintStatus(I, Q, mode);

		ultima_I = I;
		ultima_Q = Q;
		ultimo_mode = mode;
	}

	else
		{
	         // Si no responde
	    }
}

const char* MCP23017_ModeToString(SystemMode_t mode)
{
	switch (mode)
    	{
        	case MODE_RUN:    return "RUN";
        	case MODE_STOP:   return "STOP";
        	case MODE_ERROR:  return "ERROR";
        	case MODE_CARGA:  return "CARGA";
        	default:          return "UNKNOWN";
    	}
}

void MCP23017_PrintStatus(uint8_t inputs, uint8_t outputs, SystemMode_t mode)
{
    UART_Print("\033[2J\033[H");
    UART_Print("\r\n          1 2 3 4 5 6 7 8\r\n");
    UART_Print("Entradas: ");

    for (int i = 0; i < 8; i++)
    {
        UART_Print((inputs & (1 << i)) ? "1 " : "0 ");
    }

    UART_Print("\r\nSalidas : ");

    for (int i = 0; i < 8; i++)
    {
        UART_Print((outputs & (1 << i)) ? "1 " : "0 ");
    }

    UART_Print("\r\nModo    : ");
    UART_Print(MCP23017_ModeToString(mode));
    UART_Print("\r\n");
}





/*
void UART_Print_U32(uint32_t num)
{
    char buf[11];          // máx 10 dígitos + '\0'
    int i = 10;

    buf[10] = '\0';

    // caso especial 0
    if (num == 0)
    {
        UART_Print("0");
        return;
    }

    // convertir número a string (de atrás hacia delante)
    while (num > 0)
    {
        buf[--i] = (num % 10) + '0';
        num /= 10;
    }

    UART_Print(&buf[i]);
}
*/


/*
void UART_Print_demo(void)
{

    UART_Print("\r\nDemo: ");
    UART_Print_U32(cont_serial);

    cont_serial++ ;  // se incrementa cada vez que se ejecuta la función
}
*/




