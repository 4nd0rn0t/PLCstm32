/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include <config.h>
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */


//#include <string.h>

#include "bootloader.h"

#include "mcp23017.h"
#include "terminal.h"

#include "plc_nucleo.h"
#include "plc_flash.h"
#include "plc_app.h"


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */




/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

//UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */





/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */

void GPIO_Init(void);

void UART1_Init(void);
void UART1_EnableIRQ(void);

void I2C1_Init(void);


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */







/* =========================================================
			ESTADO GLOBAL
========================================================= */

void execute_block(Block16 *b, uint8_t i);


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */



  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */


  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */

 SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */

 GPIO_Init();
 UART1_Init();




  while (1)
  {
      while (!(USART1->ISR & USART_ISR_TXE_TXFNF));
      USART1->TDR = 'A';
      for (volatile int i = 0; i < 200000; i++);
  }






  /* USER CODE BEGIN 2 */

  I2C1_Init();

//  UART1_EnableIRQ();

  system_start();     // decide BOOT o PLC (bloqueante)


 // SystemCoreClockUpdate();
//  SysTick_Config(SystemCoreClock / 1000);
 // __enable_irq();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  	    while (1)
  	    {
  	    	// Por seguridad (No debería llegar aquí)
		}

}

void plc_run(void)
{

	MCP23017_Init();
	MCP23017_Check();

	plc_init();
	rng_init();

    while (1)
    {


    	if (system_flags & MCP23017_OK_FLAG)


    		     {
    		         // El chip responde

    			  actualizar_modo();
    			  parpadeo_led();

    			  switch (mode)

    			  	  {
    			         case MODE_STOP:

    			        	 plc_init();
    			        	// I = MCP23017_ReadPortB();   // Actualizar lectura de entradas

    			        	 uint8_t value = 0;
    			        	 MCP23017_ReadPortB(&value);
    			        	 I = value;


    			        	 Q = 0;
    			        	 MCP23017_WritePortA(Q);

    			             break;

    			         case MODE_RUN:

    			        	// I = MCP23017_ReadPortB();   // Actualizar lectura de entradas

    			        	// uint8_t value = 0;

    			        	 MCP23017_ReadPortB(&value);
    			        	 I = value;

    			        	 plc_scan();
    			        	 MCP23017_WritePortA(Q);

    			        	 break;

    			         case MODE_CARGA:

    			        	 plc_init();
    			        	 Q = 0;
    			        	 MCP23017_WritePortA(Q);

    			        	 // aquí puedes permitir escritura Flash / carga de programa

    			        	// NVIC_SystemReset();    // Hace reset

    			        	 break;

    			         case MODE_ERROR:
    						 break;

    			         default:
    						 break;

    			  	  }

    		     }

    		 else

    		     {
    		         // Si no responde
    			 	 mode = MODE_ERROR;
    			 	 parpadeo_led();

    		     }


    		  Terminal_Imprimir(I, Q, mode);
    }

//}




    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */













  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */


/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */

/*
static void MX_USART1_UART_Init(void)
{
*/
  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
	/*
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }

  */
  /* USER CODE BEGIN USART1_Init 2 */




  /* USER CODE END USART1_Init 2 */



/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */

/*
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  */
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */

/*
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

*/
  /*Configure GPIO pin Output Level */

/*
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
*/
  /*Configure GPIO pin : Button_Pin */
/*
  GPIO_InitStruct.Pin = Button_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Button_GPIO_Port, &GPIO_InitStruct);
*/

  /*Configure GPIO pins : PA2 PA3 */
/*
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF1_USART2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
*/
  /*Configure GPIO pins : PA4 PA5 PA6 PA7 */
/*
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF0_SPI1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
*/
  /*Configure GPIO pin : Pulsador_Pin */
/*
  GPIO_InitStruct.Pin = Pulsador_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Pulsador_GPIO_Port, &GPIO_InitStruct);
*/
  /*Configure GPIO pin : LED_Pin */
/*
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);
*/
  /*Configure GPIO pin : PB7 */
/*
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF14_I2C1;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
*/

  /*Configure GPIO pin : PC14 */
/*
  GPIO_InitStruct.Pin = GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF14_I2C1;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
*/

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
/*
}
*/

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
