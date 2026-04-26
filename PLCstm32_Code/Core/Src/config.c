/*
 * plc_config.c
 *
 *  Created on: 29 mar 2026
 *      Author: Yo
 */

#include <config.h>
#include "main.h"

#include "stm32c0xx.h"

/* =========================================================
			Definición
========================================================= */

volatile uint8_t system_flags = 0;

volatile uint32_t I = 0;
volatile uint32_t Q = 0;
volatile uint64_t M = 0;

volatile SystemMode_t mode = MODE_STOP;


uint32_t GetPCLK2Freq(void);


void SystemClock_Config(void)
{


	 // Encender HSI
	    RCC->CR |= RCC_CR_HSION;
	    while (!(RCC->CR & RCC_CR_HSIRDY));

	    // Seleccionar HSI como SYSCLK
	    RCC->CFGR &= ~RCC_CFGR_SW;

	    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);

	    SystemCoreClockUpdate();
	}












void GPIO_Init(void)
{

    RCC->IOPENR |= RCC_IOPENR_GPIOAEN | RCC_IOPENR_GPIOBEN | RCC_IOPENR_GPIOCEN | RCC_IOPENR_GPIOFEN;	// Enable clocks

    // ========================
    // LED PB6
    // ========================

    GPIOB->BSRR = GPIO_BSRR_BS6;   													// apagar LED (poner HIGH porque es active-low)
    GPIOB->MODER = (GPIOB->MODER & ~GPIO_MODER_MODE6_Msk) | GPIO_MODER_MODE6_0;  	// configurar como salida
    GPIOB->OTYPER &= ~GPIO_OTYPER_OT6_Msk;											// push-pull
    GPIOB->PUPDR &= ~GPIO_PUPDR_PUPD6_Msk;											// no pull
    GPIOB->OSPEEDR = (GPIOB->OSPEEDR & ~GPIO_OSPEEDR_OSPEED6_Msk) | GPIO_OSPEEDR_OSPEED6_1;	// high speed

    // ========================
    // Pulsador PA8
    // ========================
    GPIOA->MODER &= ~GPIO_MODER_MODE8_Msk;
    GPIOA->PUPDR = (GPIOA->PUPDR & ~GPIO_PUPDR_PUPD8_Msk) | GPIO_PUPDR_PUPD8_0;

}




void I2C1_Init(void)
 {

	 /* CLOCKS */
	    RCC->IOPENR |= RCC_IOPENR_GPIOBEN | RCC_IOPENR_GPIOCEN;
	    RCC->APBENR1 |= RCC_APBENR1_I2C1EN;

	    /* RESET */
	    RCC->APBRSTR1 |= RCC_APBRSTR1_I2C1RST;
	    RCC->APBRSTR1 &= ~RCC_APBRSTR1_I2C1RST;

	    /* GPIO PB7 SCL / PC14 SDA */
	    GPIOB->MODER |= (2U << (7*2));
	    GPIOB->OTYPER |= (1U << 7);
	    GPIOB->PUPDR |= (1U << (7*2));
	    GPIOB->AFR[0] |= (6U << (7*4));

	    GPIOC->MODER |= (2U << (14*2));
	    GPIOC->OTYPER |= (1U << 14);
	    GPIOC->PUPDR |= (1U << (14*2));
	    GPIOC->AFR[1] |= (6U << ((14-8)*4));

	    /* Disable peripheral */
	    I2C1->CR1 &= ~I2C_CR1_PE;

	    /* TIMING (100kHz típico correcto C0 @48MHz) */
	    I2C1->TIMINGR = 0x00303D5B;

	    /* FILTERS */
	    I2C1->CR1 &= ~I2C_CR1_ANFOFF;
	    I2C1->CR1 &= ~(0xF << I2C_CR1_DNF_Pos);

	    I2C1->ICR = 0xFFFFFFFF;

	    /* ENABLE */
	    I2C1->CR1 |= I2C_CR1_PE;

}




void UART1_Init(void)
{
    /* ========= CLOCKS ========= */
    RCC->IOPENR  |= RCC_IOPENR_GPIOAEN;		// GPIOA
    RCC->APBENR2 |= RCC_APBENR2_USART1EN;	// USART1

    /* RESET USART */
    RCC->APBRSTR2 |= RCC_APBRSTR2_USART1RST;
    RCC->APBRSTR2 &= ~RCC_APBRSTR2_USART1RST;

    /* CLOCK SOURCE = PCLK */
   // RCC->CCIPR &= ~(3U << RCC_CCIPR_USART1SEL_Pos);
  //  RCC->CCIPR |=  (0U << RCC_CCIPR_USART1SEL_Pos);

    RCC->CCIPR &= ~(3U << RCC_CCIPR_USART1SEL_Pos);

    /* ========= GPIO ========= */

    MODIFY_REG(GPIOA->MODER,
               GPIO_MODER_MODE9_Msk | GPIO_MODER_MODE10_Msk,
               GPIO_MODER_MODE9_1 | GPIO_MODER_MODE10_1);

    MODIFY_REG(GPIOA->AFR[1],
               GPIO_AFRH_AFSEL9_Msk | GPIO_AFRH_AFSEL10_Msk,
               (1U << GPIO_AFRH_AFSEL9_Pos) | (1U << GPIO_AFRH_AFSEL10_Pos));

    MODIFY_REG(GPIOA->PUPDR,
               GPIO_PUPDR_PUPD10_Msk,
               GPIO_PUPDR_PUPD10_0);


    /*
    // PA9 TX / PA10 RX
    GPIOA->MODER &= ~((3U << (9*2)) | (3U << (10*2)));
    GPIOA->MODER |=  ((2U << (9*2)) | (2U << (10*2))); // AF mode

    GPIOA->AFR[1] &= ~((0xFU << 4) | (0xFU << 8));
    GPIOA->AFR[1] |=  ((1U << 4) | (1U << 8)); // AF1 USART1

    GPIOA->PUPDR &= ~(3U << (10*2));
    GPIOA->PUPDR |=  (1U << (10*2)); // RX pull-up
*/

    /* ========= UART RESET STATE ========= */
    USART1->CR1 = 0;
    USART1->CR2 = 0;
    USART1->CR3 = 0;

    /* DESACTIVAR FIFO */
    USART1->CR1 &= ~USART_CR1_FIFOEN;

    // LIMPIAR FLAGS (
    USART1->ICR = 0xFFFFFFFF;

    /* BAUD */
    USART1->BRR = SystemCoreClock / 115200;


    // Enable RX + TX
    USART1->CR1 |= USART_CR1_RE | USART_CR1_TE;


    // Enable interrupt RX
    USART1->CR1 |= USART_CR1_RXNEIE_RXFNEIE;


    USART1->CR1 |= USART_CR1_UE; // USART enable

    // NVIC
    NVIC_EnableIRQ(USART1_IRQn);

}




void UART1_EnableIRQ(void)
{
 	USART1->CR1 |= USART_CR1_RXNEIE_RXFNEIE;  	 // habilitar interrupción RX
	NVIC_EnableIRQ(USART1_IRQn);		 		 // habilitar NVIC
}




volatile uint32_t msTicks = 0;

uint32_t millis(void)
{
    return msTicks;
}


void delay_ms(uint32_t ms)
{
    uint32_t start = millis();
    while ((millis() - start) < ms);
}







