/*
 * mcp23017.c
 *
 *  Created on: 28 mar 2026
 *      Author: Yo
 */

#include "main.h"
#include "plc_config.h"
#include "mcp23017.h"


// falta ver si leer bien las entradas


#define I2C_TIMEOUT  1000


static int I2C_WaitBusy(uint32_t timeout);
static int I2C_WaitTX(uint32_t timeout);
static int I2C_WaitRX(uint32_t timeout);
static int I2C_WaitSTOP(uint32_t timeout);


//--------------------------------
// ESPERAR BUS LIBRE
//--------------------------------
static int I2C_WaitBusy(uint32_t timeout)
{
    while (I2C1->ISR & I2C_ISR_BUSY)
    {
        if (timeout-- == 0)
        {
            // reset periférico I2C
            I2C1->CR1 &= ~I2C_CR1_PE;
            I2C1->CR1 |= I2C_CR1_PE;
            return -1;
        }
    }
    return 0;
}


//--------------------------------
// ESPERAR RX
//--------------------------------
static int I2C_WaitRX(uint32_t timeout)
{
    while (!(I2C1->ISR & I2C_ISR_RXNE))
    {
        // opcional: detectar NACK
        if (I2C1->ISR & I2C_ISR_NACKF)
        {
            I2C1->ICR |= I2C_ICR_NACKCF;
            I2C1->CR2 |= I2C_CR2_STOP;
            return -1;
        }

        if (timeout-- == 0)
            return -2;
    }
    return 0;
}

//--------------------------------
// ESPERAR TX
//--------------------------------
static int I2C_WaitTX(uint32_t timeout)
{
    while (!(I2C1->ISR & I2C_ISR_TXIS))
    {
        if (I2C1->ISR & I2C_ISR_NACKF)
        {
            I2C1->ICR |= I2C_ICR_NACKCF;
            I2C1->CR2 |= I2C_CR2_STOP;
            return -1;
        }

        if (timeout-- == 0)
            return -2;
    }
    return 0;
}

//--------------------------------
// ESPERAR STOP
//--------------------------------
static int I2C_WaitSTOP(uint32_t timeout)
{
    while (!(I2C1->ISR & I2C_ISR_STOPF))
    {
        if (timeout-- == 0)
            return -1;
    }

    I2C1->ICR |= I2C_ICR_STOPCF; // limpiar flag
    return 0;
}



//--------------------------------
// WRITE (reg + dato)
//--------------------------------
static int I2C_WriteReg(uint8_t addr, uint8_t reg, uint8_t value)
{
    if (I2C_WaitBusy(I2C_TIMEOUT)) return -1;

    I2C1->CR2 =
        (addr << 1) |
        (2 << I2C_CR2_NBYTES_Pos) |
        I2C_CR2_START;

    if (I2C_WaitTX(I2C_TIMEOUT)) return -1;
    I2C1->TXDR = reg;

    if (I2C_WaitTX(I2C_TIMEOUT)) return -1;
    I2C1->TXDR = value;

    I2C1->CR2 |= I2C_CR2_STOP;

    if (I2C_WaitSTOP(I2C_TIMEOUT)) return -1;

    return 0;
}

//--------------------------------
// READ (1 byte)
//--------------------------------
static int I2C_ReadReg(uint8_t addr, uint8_t reg, uint8_t *value)
{
    if (I2C_WaitBusy(I2C_TIMEOUT)) return -1;

    // escribir dirección de registro
    I2C1->CR2 =
        (addr << 1) |
        (1 << I2C_CR2_NBYTES_Pos) |
        I2C_CR2_START;

    if (I2C_WaitTX(I2C_TIMEOUT)) return -1;
    I2C1->TXDR = reg;

    if (I2C_WaitSTOP(I2C_TIMEOUT)) return -1;

    // leer dato
    I2C1->CR2 =
        (addr << 1) |
        (1 << I2C_CR2_NBYTES_Pos) |
        I2C_CR2_RD_WRN |
        I2C_CR2_START;

    if (I2C_WaitRX(I2C_TIMEOUT)) return -1;
    *value = I2C1->RXDR;

    I2C1->CR2 |= I2C_CR2_STOP;

    if (I2C_WaitSTOP(I2C_TIMEOUT)) return -1;

    return 0;
}

//--------------------------------
// 			INIT
//--------------------------------
void MCP23017_Init(void)
{
    I2C_WriteReg(MCP23017_ADDR, 0x00, 0x00); // IODIRA salida
    I2C_WriteReg(MCP23017_ADDR, 0x01, 0xFF); // IODIRB entrada
    I2C_WriteReg(MCP23017_ADDR, 0x0D, 0xFF); // GPPUB pull-up
}

//--------------------------------
// 			WRITE PORT A
//--------------------------------
int MCP23017_WritePortA(uint8_t value)
{
    int ret = I2C_WriteReg(MCP23017_ADDR, 0x12, value);

    if (ret != 0)
        system_flags &= ~MCP23017_OK_FLAG;
    else
        system_flags |= MCP23017_OK_FLAG;

    return ret;
}

//--------------------------------
// 			READ PORT B
//--------------------------------
int MCP23017_ReadPortB(uint8_t *value)
{

    int ret = I2C_ReadReg(MCP23017_ADDR, 0x13, value);

    if (ret != 0)
        system_flags &= ~MCP23017_OK_FLAG;
    else
        system_flags |= MCP23017_OK_FLAG;

    return ret;
}


//--------------------------------
// CHECK DEVICE
//--------------------------------
uint8_t MCP23017_Check(void)
{
    if (I2C_WaitBusy(I2C_TIMEOUT)) return 0;

    I2C1->CR2 =
        (MCP23017_ADDR << 1) |
        I2C_CR2_START |
        I2C_CR2_AUTOEND;

    while (!(I2C1->ISR & (I2C_ISR_STOPF | I2C_ISR_NACKF)));

    if (I2C1->ISR & I2C_ISR_NACKF)
    {
        I2C1->ICR |= I2C_ICR_NACKCF;
        return 0;
    }

    I2C1->ICR |= I2C_ICR_STOPCF;
    return 1;
}




/*
extern I2C_HandleTypeDef hi2c1;

void MCP23017_Init(void)
{
	uint8_t data[2];

    data[0] = MCP23017_IODIRA;
    data[1] = 0x00; 					// PORTA salida
    HAL_I2C_Master_Transmit(&hi2c1, MCP23017_ADDR, data, 2, 100);

    data[0] = MCP23017_IODIRB;
    data[1] = 0xFF; 					// PORTB entrada
    HAL_I2C_Master_Transmit(&hi2c1, MCP23017_ADDR, data, 2, 100);

    data[0] = MCP23017_GPPUB; 		// Pull-ups PORTB
    data[1] = 0xFF;
    HAL_I2C_Master_Transmit(&hi2c1, MCP23017_ADDR, data, 2, 100);
}


void MCP23017_WritePortA(uint8_t value)
{
	uint8_t data[2];

    data[0] = MCP23017_GPIOA;
    data[1] = value;

    HAL_I2C_Master_Transmit(&hi2c1, MCP23017_ADDR, data, 2, 100);
}

uint8_t MCP23017_ReadPortB(void)
{
    uint8_t reg = MCP23017_GPIOB;
    uint8_t value = 0;

    HAL_I2C_Master_Transmit(&hi2c1, MCP23017_ADDR, &reg, 1, 100);
 //   HAL_I2C_Master_Receive(&hi2c1, MCP23017_ADDR, &value, 1, 100);
    if (HAL_I2C_Master_Receive(&hi2c1, MCP23017_ADDR, &value, 1, 100) == HAL_ERROR)
    	{
    	system_flags &= ~MCP23017_OK_FLAG; // Si hay error borrar flag
    	}

    else
    	{

    	}

  return value;
}

void MCP23017_Check(void)
{
	if (HAL_I2C_IsDeviceReady(&hi2c1, MCP23017_ADDR, 3, 100) == HAL_OK)
    	{
			system_flags |= MCP23017_OK_FLAG;   // Si responde Flag a 1
    	}

	else
       {
    	   system_flags &= ~MCP23017_OK_FLAG;   // No responde Flag a 0
       }
}

*/

