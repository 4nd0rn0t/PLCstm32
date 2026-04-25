/*
 * mcp23017.c
 *
 *  Created on: 28 mar 2026
 *      Author: Yo
 */

#include <config.h>
#include "main.h"
#include "mcp23017.h"


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
          //  I2C1->CR1 &= ~I2C_CR1_PE;
           // I2C1->CR1 |= I2C_CR1_PE;
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

    // 1. Enviar registro (WRITE)
    I2C1->CR2 =
        (addr) |
        (1 << I2C_CR2_NBYTES_Pos) |
        I2C_CR2_START;

    if (I2C_WaitTX(I2C_TIMEOUT)) return -1;
    I2C1->TXDR = reg;

    if (I2C_WaitTX(I2C_TIMEOUT)) return -1;

    // 2. REPEATED START + READ
    I2C1->CR2 =
        (addr) |
        (1 << I2C_CR2_NBYTES_Pos) |
        I2C_CR2_RD_WRN |
        I2C_CR2_START;

    if (I2C_WaitRX(I2C_TIMEOUT)) return -1;
    *value = I2C1->RXDR;

    if (I2C_WaitSTOP(I2C_TIMEOUT)) return -1;

    return 0;
}

//--------------------------------
// 			INIT
//--------------------------------
void MCP23017_Init(void)
{
    I2C_WriteReg(MCP23017_ADDR, MCP23017_IODIRA, 0x00); // IODIRA salida
    I2C_WriteReg(MCP23017_ADDR, MCP23017_IODIRB, 0xFF); // IODIRB entrada
    I2C_WriteReg(MCP23017_ADDR, MCP23017_GPPUB, 0xFF); // GPPUB pull-up
}

//--------------------------------
// 			WRITE PORT A
//--------------------------------
int MCP23017_WritePortA(uint8_t value)
{
    int ret = I2C_WriteReg(MCP23017_ADDR, MCP23017_GPIOA, value);

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

    int ret = I2C_ReadReg(MCP23017_ADDR, MCP23017_GPIOB, value);

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

    // limpiar flags antiguos por seguridad
    I2C1->ICR = I2C_ICR_NACKCF | I2C_ICR_STOPCF;

    // configurar transferencia (WRITE probe)
    I2C1->CR2 =
        (MCP23017_ADDR << 1) |
        (1 << I2C_CR2_NBYTES_Pos) |   // mínimo 1 byte ficticio
        I2C_CR2_AUTOEND |
        I2C_CR2_START;

    // esperar fin
    while (!(I2C1->ISR & (I2C_ISR_STOPF | I2C_ISR_NACKF)));

    if (I2C1->ISR & I2C_ISR_NACKF)
    {
        I2C1->ICR = I2C_ICR_NACKCF | I2C_ICR_STOPCF;
        return 0;
    }

    I2C1->ICR = I2C_ICR_STOPCF;
    return 1;
}


