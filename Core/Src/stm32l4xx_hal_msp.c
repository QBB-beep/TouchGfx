/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : stm32l4xx_hal_msp.c
  * Description        : This file provides code for the MSP Initialization 
  *                      and de-Initialization codes.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN Define */
 
/* USER CODE END Define */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN Macro */

/* USER CODE END Macro */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* External functions --------------------------------------------------------*/
/* USER CODE BEGIN ExternalFunctions */

/* USER CODE END ExternalFunctions */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * Initializes the Global MSP.
  */
void HAL_MspInit(void)
{
  /* USER CODE BEGIN MspInit 0 */

  /* USER CODE END MspInit 0 */

  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();

  /* System interrupt init*/
  /* PendSV_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(PendSV_IRQn, 15, 0);

  /* USER CODE BEGIN MspInit 1 */

  /* USER CODE END MspInit 1 */
}

/**
* @brief CRC MSP Initialization
* This function configures the hardware resources used in this example
* @param hcrc: CRC handle pointer
* @retval None
*/
void HAL_CRC_MspInit(CRC_HandleTypeDef* hcrc)
{
  if(hcrc->Instance==CRC)
  {
  /* USER CODE BEGIN CRC_MspInit 0 */

  /* USER CODE END CRC_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_CRC_CLK_ENABLE();
  /* USER CODE BEGIN CRC_MspInit 1 */

  /* USER CODE END CRC_MspInit 1 */
  }

}

/**
* @brief CRC MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hcrc: CRC handle pointer
* @retval None
*/
void HAL_CRC_MspDeInit(CRC_HandleTypeDef* hcrc)
{
  if(hcrc->Instance==CRC)
  {
  /* USER CODE BEGIN CRC_MspDeInit 0 */

  /* USER CODE END CRC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CRC_CLK_DISABLE();
  /* USER CODE BEGIN CRC_MspDeInit 1 */

  /* USER CODE END CRC_MspDeInit 1 */
  }

}

/**
* @brief DMA2D MSP Initialization
* This function configures the hardware resources used in this example
* @param hdma2d: DMA2D handle pointer
* @retval None
*/
void HAL_DMA2D_MspInit(DMA2D_HandleTypeDef* hdma2d)
{
  if(hdma2d->Instance==DMA2D)
  {
  /* USER CODE BEGIN DMA2D_MspInit 0 */

  /* USER CODE END DMA2D_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_DMA2D_CLK_ENABLE();
    /* DMA2D interrupt Init */
    HAL_NVIC_SetPriority(DMA2D_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA2D_IRQn);
  /* USER CODE BEGIN DMA2D_MspInit 1 */

  /* USER CODE END DMA2D_MspInit 1 */
  }

}

/**
* @brief DMA2D MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hdma2d: DMA2D handle pointer
* @retval None
*/
void HAL_DMA2D_MspDeInit(DMA2D_HandleTypeDef* hdma2d)
{
  if(hdma2d->Instance==DMA2D)
  {
  /* USER CODE BEGIN DMA2D_MspDeInit 0 */

  /* USER CODE END DMA2D_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_DMA2D_CLK_DISABLE();

    /* DMA2D interrupt DeInit */
    HAL_NVIC_DisableIRQ(DMA2D_IRQn);
  /* USER CODE BEGIN DMA2D_MspDeInit 1 */

  /* USER CODE END DMA2D_MspDeInit 1 */
  }

}

/**
* @brief DSI MSP Initialization
* This function configures the hardware resources used in this example
* @param hdsi: DSI handle pointer
* @retval None
*/
void HAL_DSI_MspInit(DSI_HandleTypeDef* hdsi)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hdsi->Instance==DSI)
  {
  /* USER CODE BEGIN DSI_MspInit 0 */
  /* Enable HSE used for DSI PLL */
  RCC_OscInitTypeDef RCC_OscInitStruct;
  HAL_RCC_GetOscConfig(&RCC_OscInitStruct);
  if(RCC_OscInitStruct.HSEState == RCC_HSE_OFF)
  {
      RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
      RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
      RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_NONE;
      if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
      {
          while(1);
      }
  }
  /* USER CODE END DSI_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_DSI_CLK_ENABLE();

    __HAL_RCC_GPIOF_CLK_ENABLE();
    /**DSIHOST GPIO Configuration
    PF11     ------> DSIHOST_TE
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF11_DSI;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    /* DSI interrupt Init */
    HAL_NVIC_SetPriority(DSI_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DSI_IRQn);
  /* USER CODE BEGIN DSI_MspInit 1 */

  /* USER CODE END DSI_MspInit 1 */
  }

}

/**
* @brief DSI MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hdsi: DSI handle pointer
* @retval None
*/
void HAL_DSI_MspDeInit(DSI_HandleTypeDef* hdsi)
{
  if(hdsi->Instance==DSI)
  {
  /* USER CODE BEGIN DSI_MspDeInit 0 */

  /* USER CODE END DSI_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_DSI_CLK_DISABLE();

    /**DSIHOST GPIO Configuration
    PF11     ------> DSIHOST_TE
    */
    HAL_GPIO_DeInit(GPIOF, GPIO_PIN_11);

    /* DSI interrupt DeInit */
    HAL_NVIC_DisableIRQ(DSI_IRQn);
  /* USER CODE BEGIN DSI_MspDeInit 1 */

  /* USER CODE END DSI_MspDeInit 1 */
  }

}

/**
* @brief GFXMMU MSP Initialization
* This function configures the hardware resources used in this example
* @param hgfxmmu: GFXMMU handle pointer
* @retval None
*/
void HAL_GFXMMU_MspInit(GFXMMU_HandleTypeDef* hgfxmmu)
{
  if(hgfxmmu->Instance==GFXMMU)
  {
  /* USER CODE BEGIN GFXMMU_MspInit 0 */

  /* USER CODE END GFXMMU_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_GFXMMU_CLK_ENABLE();
  /* USER CODE BEGIN GFXMMU_MspInit 1 */

  /* USER CODE END GFXMMU_MspInit 1 */
  }

}

/**
* @brief GFXMMU MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hgfxmmu: GFXMMU handle pointer
* @retval None
*/
void HAL_GFXMMU_MspDeInit(GFXMMU_HandleTypeDef* hgfxmmu)
{
  if(hgfxmmu->Instance==GFXMMU)
  {
  /* USER CODE BEGIN GFXMMU_MspDeInit 0 */

  /* USER CODE END GFXMMU_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_GFXMMU_CLK_DISABLE();
  /* USER CODE BEGIN GFXMMU_MspDeInit 1 */

  /* USER CODE END GFXMMU_MspDeInit 1 */
  }

}

/**
* @brief I2C MSP Initialization
* This function configures the hardware resources used in this example
* @param hi2c: I2C handle pointer
* @retval None
*/
void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hi2c->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspInit 0 */

  /* USER CODE END I2C1_MspInit 0 */

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    HAL_PWREx_EnableVddIO2();
    /**I2C1 GPIO Configuration
    PB6     ------> I2C1_SCL
    PG13     ------> I2C1_SDA
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    /* Peripheral clock enable */
    __HAL_RCC_I2C1_CLK_ENABLE();
    /* I2C1 interrupt Init */
    HAL_NVIC_SetPriority(I2C1_EV_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
    HAL_NVIC_SetPriority(I2C1_ER_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
  /* USER CODE BEGIN I2C1_MspInit 1 */

  /* USER CODE END I2C1_MspInit 1 */
  }

}

/**
* @brief I2C MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hi2c: I2C handle pointer
* @retval None
*/
void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c)
{
  if(hi2c->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspDeInit 0 */

  /* USER CODE END I2C1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_I2C1_CLK_DISABLE();

    /**I2C1 GPIO Configuration
    PB6     ------> I2C1_SCL
    PG13     ------> I2C1_SDA
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6);

    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_13);

    /* I2C1 interrupt DeInit */
    HAL_NVIC_DisableIRQ(I2C1_EV_IRQn);
    HAL_NVIC_DisableIRQ(I2C1_ER_IRQn);
  /* USER CODE BEGIN I2C1_MspDeInit 1 */

  /* USER CODE END I2C1_MspDeInit 1 */
  }

}

/**
* @brief LTDC MSP Initialization
* This function configures the hardware resources used in this example
* @param hltdc: LTDC handle pointer
* @retval None
*/
void HAL_LTDC_MspInit(LTDC_HandleTypeDef* hltdc)
{
  if(hltdc->Instance==LTDC)
  {
  /* USER CODE BEGIN LTDC_MspInit 0 */

  /* USER CODE END LTDC_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_LTDC_CLK_ENABLE();
    /* LTDC interrupt Init */
    HAL_NVIC_SetPriority(LTDC_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(LTDC_IRQn);
    HAL_NVIC_SetPriority(LTDC_ER_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(LTDC_ER_IRQn);
  /* USER CODE BEGIN LTDC_MspInit 1 */

  /* USER CODE END LTDC_MspInit 1 */
  }

}

/**
* @brief LTDC MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hltdc: LTDC handle pointer
* @retval None
*/
void HAL_LTDC_MspDeInit(LTDC_HandleTypeDef* hltdc)
{
  if(hltdc->Instance==LTDC)
  {
  /* USER CODE BEGIN LTDC_MspDeInit 0 */

  /* USER CODE END LTDC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_LTDC_CLK_DISABLE();

    /* LTDC interrupt DeInit */
    HAL_NVIC_DisableIRQ(LTDC_IRQn);
    HAL_NVIC_DisableIRQ(LTDC_ER_IRQn);
  /* USER CODE BEGIN LTDC_MspDeInit 1 */

  /* USER CODE END LTDC_MspDeInit 1 */
  }

}

/**
* @brief OSPI MSP Initialization
* This function configures the hardware resources used in this example
* @param hospi: OSPI handle pointer
* @retval None
*/
void HAL_OSPI_MspInit(OSPI_HandleTypeDef* hospi)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hospi->Instance==OCTOSPI1)
  {
  /* USER CODE BEGIN OCTOSPI1_MspInit 0 */
  __HAL_RCC_OSPIM_CLK_ENABLE();
  __HAL_RCC_OSPI1_CLK_ENABLE();

  /* Reset the OctoSPI memory interface */
  __HAL_RCC_OSPI1_FORCE_RESET();
  __HAL_RCC_OSPI1_RELEASE_RESET();
  /* USER CODE END OCTOSPI1_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_OSPIM_CLK_ENABLE();
    __HAL_RCC_OSPI1_CLK_ENABLE();

    __HAL_RCC_GPIOI_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    HAL_PWREx_EnableVddIO2();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    /**OCTOSPI1 GPIO Configuration
    PI10     ------> OCTOSPIM_P2_IO1
    PI9     ------> OCTOSPIM_P2_IO2
    PG9     ------> OCTOSPIM_P2_IO6
    PI6     ------> OCTOSPIM_P2_CLK
    PI11     ------> OCTOSPIM_P2_IO0
    PG15     ------> OCTOSPIM_P2_DQS
    PH9     ------> OCTOSPIM_P2_IO4
    PG10     ------> OCTOSPIM_P2_IO7
    PG12     ------> OCTOSPIM_P2_NCS
    PH10     ------> OCTOSPIM_P2_IO5
    PH8     ------> OCTOSPIM_P2_IO3
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_9|GPIO_PIN_6|GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_OCTOSPIM_P2;
    HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_15|GPIO_PIN_10|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_OCTOSPIM_P2;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_OCTOSPIM_P2;
    HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /* USER CODE BEGIN OCTOSPI1_MspInit 1 */

  /* USER CODE END OCTOSPI1_MspInit 1 */
  }

}

/**
* @brief OSPI MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hospi: OSPI handle pointer
* @retval None
*/
void HAL_OSPI_MspDeInit(OSPI_HandleTypeDef* hospi)
{
  if(hospi->Instance==OCTOSPI1)
  {
  /* USER CODE BEGIN OCTOSPI1_MspDeInit 0 */

  /* USER CODE END OCTOSPI1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_OSPIM_CLK_DISABLE();
    __HAL_RCC_OSPI1_CLK_DISABLE();

    /**OCTOSPI1 GPIO Configuration
    PI10     ------> OCTOSPIM_P2_IO1
    PI9     ------> OCTOSPIM_P2_IO2
    PG9     ------> OCTOSPIM_P2_IO6
    PI6     ------> OCTOSPIM_P2_CLK
    PI11     ------> OCTOSPIM_P2_IO0
    PG15     ------> OCTOSPIM_P2_DQS
    PH9     ------> OCTOSPIM_P2_IO4
    PG10     ------> OCTOSPIM_P2_IO7
    PG12     ------> OCTOSPIM_P2_NCS
    PH10     ------> OCTOSPIM_P2_IO5
    PH8     ------> OCTOSPIM_P2_IO3
    */
    HAL_GPIO_DeInit(GPIOI, GPIO_PIN_10|GPIO_PIN_9|GPIO_PIN_6|GPIO_PIN_11);

    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_9|GPIO_PIN_15|GPIO_PIN_10|GPIO_PIN_12);

    HAL_GPIO_DeInit(GPIOH, GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_8);

  /* USER CODE BEGIN OCTOSPI1_MspDeInit 1 */

  /* USER CODE END OCTOSPI1_MspDeInit 1 */
  }

}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
