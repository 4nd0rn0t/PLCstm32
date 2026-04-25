/*
 * plc_config.c
 *
 *  Created on: 29 mar 2026
 *      Author: Yo
 */

#include <config.h>
#include "main.h"


/* =========================================================
			Definición
========================================================= */

volatile uint8_t system_flags = 0;

volatile uint32_t I = 0;
volatile uint32_t Q = 0;
volatile uint64_t M = 0;

volatile SystemMode_t mode = MODE_STOP;


/*
void SystemClock_Config(void)

	{
	    RCC->CR |= RCC_CR_HSION;
	    while (!(RCC->CR & RCC_CR_HSIRDY));

	    RCC->CFGR &= ~RCC_CFGR_SW;
	    RCC->CFGR |= (0x0 << RCC_CFGR_SW_Pos);

	    while ((RCC->CFGR & RCC_CFGR_SWS) != (0x0 << RCC_CFGR_SWS_Pos));

	    SystemCoreClockUpdate();

	    SysTick_Config(SystemCoreClock / 1000);

	    __enable_irq();
	}
*/




void SystemClock_Config(void)
{
    // 1. Enable HSI
    RCC->CR |= RCC_CR_HSION;
    while (!(RCC->CR & RCC_CR_HSIRDY));

    // 2. FORCE HSI DIV = 1 (48 MHz)
    RCC->CR &= ~RCC_CR_HSIDIV;   // limpia todo el campo

    // IMPORTANTE: algunos C0 no usan bit directo,
    // así que forzamos el valor "DIV1"
    RCC->CR |= (0x0 << RCC_CR_HSIDIV_Pos);

    // 3. Select HSI as SYSCLK
    RCC->CFGR &= ~RCC_CFGR_SW;

    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);

    // 4. Prescalers = 1
    RCC->CFGR &= ~RCC_CFGR_HPRE;
    RCC->CFGR &= ~RCC_CFGR_PPRE;

    SystemCoreClockUpdate();
}








void GPIO_Init(void)
{
    // 1. Enable clocks
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN | RCC_IOPENR_GPIOBEN | RCC_IOPENR_GPIOCEN | RCC_IOPENR_GPIOFEN;

    // ========================
    // LED PB6
    // ========================

    GPIOB->BSRR = (1UL << LED_PIN_NUM);  			// apagar LED (poner HIGH porque es active-low)

    GPIOB->MODER &= ~(3UL << (LED_PIN_NUM  * 2U));
    GPIOB->MODER |=  (1UL << (LED_PIN_NUM  * 2U));  // configurar como salida

    GPIOB->OTYPER &= ~(1UL << LED_PIN_NUM );		// push-pull
    GPIOB->PUPDR  &= ~(3UL << (LED_PIN_NUM  * 2U)); // no pull

    GPIOB->OSPEEDR &= ~(3UL << (LED_PIN_NUM * 2U)); // limpiar primero
    GPIOB->OSPEEDR |=  (3UL << (LED_PIN_NUM * 2U)); // high speed

    // ========================
    // Pulsador PA8
    // ========================
    GPIOA->MODER &= ~(3UL << (PULSADOR_PIN_NUM * 2U));
    GPIOA->PUPDR &= ~(3UL << (PULSADOR_PIN_NUM * 2U));
    GPIOA->PUPDR |=  (1UL << (PULSADOR_PIN_NUM * 2U));


    // ========================
	// USART1 (PA9 TX, PA10 RX)
	// ========================

	    // PA9 -> TX
	    GPIOA->MODER &= ~(3UL << (9 * 2));
	    GPIOA->MODER |=  (2UL << (9 * 2));   // AF mode

	    GPIOA->OTYPER &= ~(1UL << 9);        // push-pull
	    GPIOA->PUPDR  &= ~(3UL << (9 * 2));  // no pull
	    GPIOA->OSPEEDR |= (3UL << (9 * 2));  // high speed

	    GPIOA->AFR[1] &= ~(0xFUL << ((9 - 8) * 4));  // limpiar
	    GPIOA->AFR[1] |=  (1UL << ((9 - 8) * 4));    // AF1 = USART1

	    // PA10 -> RX
	    GPIOA->MODER &= ~(3UL << (10 * 2));
	    GPIOA->MODER |=  (2UL << (10 * 2));  // AF mode

	    GPIOA->PUPDR  &= ~(3UL << (10 * 2));
	    GPIOA->PUPDR  |=  (1UL << (10 * 2)); // pull-up recomendable

	    GPIOA->AFR[1] &= ~(0xFUL << ((10 - 8) * 4));
	    GPIOA->AFR[1] |=  (1UL << ((10 - 8) * 4));   // AF1


	    // ========================
	    // PB7 -> I2C1_SCL
	    // ========================

	       // MODER = Alternate Function (10)
	       GPIOB->MODER &= ~(3UL << (7U * 2U));
	       GPIOB->MODER |=  (2UL << (7U * 2U));

	       // OTYPER = Open Drain (1)
	       GPIOB->OTYPER &= ~(1UL << 7U);
	       GPIOB->OTYPER |=  (1UL << 7U);

	       // PUPDR = Pull-Up (01)
	       GPIOB->PUPDR &= ~(3UL << (7U * 2U));
	       GPIOB->PUPDR |=  (1UL << (7U * 2U));

	       // SPEED (opcional pero recomendable)
	       GPIOB->OSPEEDR &= ~(3UL << (7U * 2U));
	       GPIOB->OSPEEDR |=  (2UL << (7U * 2U)); // medium/high

	       // AFR = AF6 (I2C1)
	       GPIOB->AFR[0] &= ~(0xFUL << (7U * 4U));
	       GPIOB->AFR[0] |=  (6UL << (7U * 4U));


	       // ========================
	       // PC14 -> I2C1_SDA
	       // ========================

	       // MODER = Alternate Function
	       GPIOC->MODER &= ~(3UL << (14U * 2U));
	       GPIOC->MODER |=  (2UL << (14U * 2U));

	       // OTYPER = Open Drain
	       GPIOC->OTYPER &= ~(1UL << 14U);
	       GPIOC->OTYPER |=  (1UL << 14U);

	       // PUPDR = Pull-Up
	       GPIOC->PUPDR &= ~(3UL << (14U * 2U));
	       GPIOC->PUPDR |=  (1UL << (14U * 2U));

	       // SPEED
	       GPIOC->OSPEEDR &= ~(3UL << (14U * 2U));
	       GPIOC->OSPEEDR |=  (2UL << (14U * 2U));

	       // AFR = AF6 (I2C1)
	       GPIOC->AFR[1] &= ~(0xFUL << ((14U - 8U) * 4U));
	       GPIOC->AFR[1] |=  (6UL << ((14U - 8U) * 4U));


}




void I2C1_Init(void)
    {
        // 1. Clock GPIO + I2C
        RCC->IOPENR |= RCC_IOPENR_GPIOBEN | RCC_IOPENR_GPIOCEN;
        RCC->APBENR1 |= RCC_APBENR1_I2C1EN;

        // 2. Reset I2C
        RCC->APBRSTR1 |= RCC_APBRSTR1_I2C1RST;
        RCC->APBRSTR1 &= ~RCC_APBRSTR1_I2C1RST;

        // 3. Desactivar I2C
        I2C1->CR1 &= ~I2C_CR1_PE;

        // 4. TIMING (EL TUYO DE HAL)
   //     I2C1->TIMINGR = 0x0090194B;      //400 Khz

        I2C1->TIMINGR = 0x00303D5B;		//100 Khz

        // 5. Filtros (igual que HAL)
        I2C1->CR1 &= ~I2C_CR1_ANFOFF;    		 // analog filter ON
        I2C1->CR1 &= ~(0xF << I2C_CR1_DNF_Pos);  // digital filter = 0

        // 6. Limpiar flags
        I2C1->ICR = 0xFFFFFFFF;

        // 7. Activar I2C
        I2C1->CR1 |= I2C_CR1_PE;
    }




void UART1_Init(void)
	{

		RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
	    RCC->APBENR2 |= RCC_APBENR2_USART1EN;

	    USART1->BRR = SystemCoreClock / 115200;

	    USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;

	}





void UART1_EnableIRQ(void)
{
  // habilitar interrupción RX
  USART1->CR1 |= USART_CR1_RXNEIE_RXFNEIE;  // puesto por mi

  // habilitar NVIC
  NVIC_EnableIRQ(USART1_IRQn);		// puesto por mi
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




