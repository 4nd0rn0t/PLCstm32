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
int MCP_WriteReg(uint8_t addr, uint8_t reg, uint8_t data);
int MCP_ReadReg(uint8_t addr, uint8_t reg, uint8_t *data);

void MCP23017_Init(void);
int MCP_WritePortA(uint8_t value);
uint8_t MCP23017_Check(void);



static int I2C_WaitBusy(uint32_t timeout)
{
    while (I2C1->ISR & I2C_ISR_BUSY)
    {
        if (--timeout == 0)
            return -1;
    }
    return 0;
}

int MCP_WriteReg(uint8_t addr, uint8_t reg, uint8_t data)
{
    if (I2C_WaitBusy(I2C_TIMEOUT))
        return -1;

    // START + WRITE + 2 bytes
    I2C1->CR2 =
        (addr << 1) |
        (2 << I2C_CR2_NBYTES_Pos) |
        I2C_CR2_START |
        I2C_CR2_AUTOEND;

    // enviar REG
    while (!(I2C1->ISR & I2C_ISR_TXIS))
        if (I2C1->ISR & I2C_ISR_NACKF) return -2;

    I2C1->TXDR = reg;

    // enviar DATA
    while (!(I2C1->ISR & I2C_ISR_TXIS))
        if (I2C1->ISR & I2C_ISR_NACKF) return -2;

    I2C1->TXDR = data;

    // STOP automático (AUTOEND)

    while (!(I2C1->ISR & I2C_ISR_STOPF));
    I2C1->ICR |= I2C_ICR_STOPCF;

    return 0;
}

int MCP_ReadReg(uint8_t addr, uint8_t reg, uint8_t *data)
{
    if (I2C_WaitBusy(I2C_TIMEOUT))
        return -1;

    // FASE 1: escribir registro
    I2C1->CR2 =
        (addr << 1) |
        (1 << I2C_CR2_NBYTES_Pos) |
        I2C_CR2_START;

    while (!(I2C1->ISR & I2C_ISR_TXIS))
        if (I2C1->ISR & I2C_ISR_NACKF) return -2;

    I2C1->TXDR = reg;

    while (!(I2C1->ISR & I2C_ISR_TC));

    // FASE 2: leer dato
    I2C1->CR2 =
        (addr << 1) |
        (1 << I2C_CR2_NBYTES_Pos) |
        I2C_CR2_RD_WRN |
        I2C_CR2_START |
        I2C_CR2_AUTOEND;

    while (!(I2C1->ISR & I2C_ISR_RXNE));

    *data = I2C1->RXDR;

    while (!(I2C1->ISR & I2C_ISR_STOPF));
    I2C1->ICR |= I2C_ICR_STOPCF;

    return 0;
}


void MCP23017_Init(void)
{
    MCP_WriteReg(MCP23017_ADDR, MCP23017_IODIRA, 0x00); // A Output
    MCP_WriteReg(MCP23017_ADDR, MCP23017_IODIRB, 0xFF); // B Input
    MCP_WriteReg(MCP23017_ADDR, MCP23017_GPPUB, 0xFF); // Pull-up B
}


int MCP_WritePortA(uint8_t value)
{
    return MCP_WriteReg(MCP23017_ADDR, MCP23017_GPIOA, value);
}

int MCP_ReadPortB(uint8_t *value)
{
    return MCP_ReadReg(MCP23017_ADDR, MCP23017_GPIOB, value);
}

uint8_t MCP23017_Check(void)
{
    if (I2C_WaitBusy(I2C_TIMEOUT))
        return 0;

    I2C1->ICR = I2C_ICR_NACKCF | I2C_ICR_STOPCF;

    I2C1->CR2 =
        (MCP23017_ADDR << 1) |
        I2C_CR2_START |
        I2C_CR2_AUTOEND;

    uint32_t t = I2C_TIMEOUT;

    while (t--)
    {
        if (I2C1->ISR & I2C_ISR_NACKF)
        {
            I2C1->ICR |= I2C_ICR_NACKCF;
          //  return 0;
            return system_flags &= ~MCP23017_OK_FLAG;   // No responde Flag a 0
        }

        if (I2C1->ISR & I2C_ISR_STOPF)
        {
            I2C1->ICR |= I2C_ICR_STOPCF;
            //return 1;
            return system_flags |= MCP23017_OK_FLAG;   // Si responde Flag a 1
        }
    }

   // return 0;
    return system_flags &= ~MCP23017_OK_FLAG;   // No responde Flag a 0
}























