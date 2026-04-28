/*
 * plc_config.c
 *
 *  Created on: 29 mar 2026
 *      Author: Yo
 */

#include "config.h"

#include "main.h"


/* =========================================================
			Definición
========================================================= */

volatile uint8_t system_flags = 0;

volatile uint32_t I = 0;
volatile uint32_t Q = 0;
volatile uint64_t M = 0;

volatile SystemMode_t mode = MODE_STOP;

volatile uint32_t msTicks = 0;


void SystemClock_Config(void)
{

	FLASH->ACR = (FLASH->ACR & ~FLASH_ACR_LATENCY) | FLASH_ACR_LATENCY_1;		/* 1. Configurar latencia de FLASH (NO pisar otros bits) */

	RCC->CR |= RCC_CR_HSION;
	while (!(RCC->CR & RCC_CR_HSIRDY))		// HSI ON + esperar listo
	{
	}

	RCC->ICSCR = (RCC->ICSCR & ~RCC_ICSCR_HSITRIM_Msk) | (64 << RCC_ICSCR_HSITRIM_Pos);	// Calibración HSI 64 por ejemplo

	RCC->CR &= ~RCC_CR_HSIDIV;		// HSI sin divisor (DIV1)

	RCC->CFGR &= ~RCC_CFGR_HPRE;  //  AHB prescaler = 1

  /* SYSCLK = HSI */
	RCC->CFGR &= ~(3U << RCC_CFGR_SW_Pos);
	RCC->CFGR |=  (0U << RCC_CFGR_SW_Pos);   // SYSCLK = HSI (en C0: valor = 0)

	while (((RCC->CFGR >> RCC_CFGR_SWS_Pos) & 0x3U) != 0U)
	{
	}

	RCC->CFGR &= ~RCC_CFGR_PPRE;		// APB1 prescaler = 1

	SysTick->LOAD = (48000000 / 1000) - 1;
	SysTick->VAL  = 0;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
	                 SysTick_CTRL_TICKINT_Msk   |
	                 SysTick_CTRL_ENABLE_Msk;				// Configurar SysTick a 1 ms (48 MHz)

	SystemCoreClock = 48000000;		// Actualizar variable global

}


void GPIO_Init(void)
{
    /* Clock */
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN | RCC_IOPENR_GPIOBEN | RCC_IOPENR_GPIOCEN | RCC_IOPENR_GPIOFEN;

    // ========================
    // LED PB6 (active LOW)
    // ========================

    /* Configurar como salida */
    GPIOB->MODER = (GPIOB->MODER & ~GPIO_MODER_MODE6_Msk) | GPIO_MODER_MODE6_0;

    /* Push-pull */
    GPIOB->OTYPER &= ~GPIO_OTYPER_OT6_Msk;

    /* Sin pull */
    GPIOB->PUPDR &= ~GPIO_PUPDR_PUPD6_Msk;

    /* Velocidad alta */
    GPIOB->OSPEEDR = (GPIOB->OSPEEDR & ~GPIO_OSPEEDR_OSPEED6_Msk) | GPIO_OSPEEDR_OSPEED6_1;

    /* Apagar LED (activo en LOW) */
    GPIOB->BSRR = GPIO_BSRR_BS6;

    // ========================
    // Pulsador PA8
    // ========================

    /* Input */
    GPIOA->MODER &= ~GPIO_MODER_MODE8_Msk;

    /* Pull-up */
    GPIOA->PUPDR = (GPIOA->PUPDR & ~GPIO_PUPDR_PUPD8_Msk) |
                   GPIO_PUPDR_PUPD8_0;
}


void USART1_Init(void)
{
    /* Clocks */
    RCC->IOPENR  |= RCC_IOPENR_GPIOAEN;
    RCC->APBENR2 |= RCC_APBENR2_USART1EN;

    /* PA9 TX */
    GPIOA->MODER &= ~(3U << (9 * 2));
    GPIOA->MODER |=  (2U << (9 * 2));
    GPIOA->AFR[1] &= ~(0xF << 4);
    GPIOA->AFR[1] |=  (1U << 4);
    GPIOA->OTYPER &= ~(1U << 9);
    GPIOA->PUPDR  &= ~(3U << (9 * 2));

    /* PA10 RX */
    GPIOA->MODER &= ~(3U << (10 * 2));
    GPIOA->MODER |=  (2U << (10 * 2));
    GPIOA->AFR[1] &= ~(0xF << 8);
    GPIOA->AFR[1] |=  (1U << 8);
    GPIOA->PUPDR  &= ~(3U << (10 * 2));

    /* USART config */
    USART1->CR1 = 0;
    USART1->CR2 = 0;
    USART1->CR3 = 0;

    USART1->BRR = SystemCoreClock / 115200;

    USART1->CR2 &= ~USART_CR2_STOP;

    USART1->CR1 &= ~(USART_CR1_M0 | USART_CR1_M1);
    USART1->CR1 &= ~USART_CR1_PCE;

    USART1->CR1 |= USART_CR1_TE | USART_CR1_RE;

    USART1->CR1 |= USART_CR1_UE;

    /* wait ready */
    while (!(USART1->ISR & USART_ISR_TEACK)) {}
    while (!(USART1->ISR & USART_ISR_REACK)) {}

    USART1->CR1 |= USART_CR1_RXNEIE_RXFNEIE;
    NVIC_EnableIRQ(USART1_IRQn);				// Habilitar RX interrupt

}


void I2C1_Init(void)
{
    /* Clock */
    RCC->APBENR1 |= RCC_APBENR1_I2C1EN;
    RCC->IOPENR  |= RCC_IOPENR_GPIOBEN | RCC_IOPENR_GPIOCEN;

    /* PB7 SCL AF14 open-drain */
    GPIOB->MODER &= ~(3U << (7*2));
    GPIOB->MODER |=  (2U << (7*2));
    GPIOB->OTYPER |= (1U << 7);
    GPIOB->AFR[0] &= ~(0xFU << (7*4));
    GPIOB->AFR[0] |=  (14U << (7*4));

    /* PC14 SDA AF14 open-drain */
    GPIOC->MODER &= ~(3U << (14*2));
    GPIOC->MODER |=  (2U << (14*2));
    GPIOC->OTYPER |= (1U << 14);
    GPIOC->AFR[1] &= ~(0xFU << ((14-8)*4));
    GPIOC->AFR[1] |=  (14U << ((14-8)*4));

    /* Disable peripheral */
    I2C1->CR1 &= ~I2C_CR1_PE;

    /* Timing (CRÍTICO) */
   // I2C1->TIMINGR = 0x0090194B;		// 100 Khz
    I2C1->TIMINGR = 0x00300F38;		// 400 Khz

    /* Enable I2C */
    I2C1->CR1 |= I2C_CR1_PE;
}







