/**
  ******************************************************************************
  * File Name          : STM32TouchController.cpp
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* USER CODE BEGIN STM32TouchController */
#include <STM32TouchController.hpp>
#include <ft3x67.h>
#include <stm32l4xx.h>
#include "cmsis_os.h"

#define I2C1_TIMEOUT_MAX        3000
#define SQRT_2                  1.4142135623730950488016887242097f /* sqrt(2) */
#define TOUCHQUEUEOBJECTS       1
osMessageQueueId_t mid_MsgQueue;
extern I2C_HandleTypeDef hi2c1;

typedef struct
{
    uint16_t x;
    uint16_t y;
} TouchQueue_t;

extern osSemaphoreId_t touchBinarySemHandle;

void STM32TouchController::init()
{
    /**
     * Initialize touch controller and driver
     *
     */
    ft3x67_ts_drv.Init(TS_I2C_ADDRESS);
    /* Configure and Start the TouchScreen driver */
    ft3x67_ts_drv.Start(TS_I2C_ADDRESS);
    ft3x67_ts_drv.EnableIT(TS_I2C_ADDRESS);

    mid_MsgQueue = osMessageQueueNew(TOUCHQUEUEOBJECTS, sizeof(TouchQueue_t), NULL);
}

bool STM32TouchController::sampleTouch(int32_t& x, int32_t& y)
{
    /**
     * By default sampleTouch returns false,
     * return true if a touch has been detected, otherwise false.
     *
     * Coordinates are passed to the caller by reference by x and y.
     *
     * This function is called by the TouchGFX framework.
     * By default sampleTouch is called every tick, this can be adjusted by HAL::setTouchSampleRate(int8_t);
     *
     */
    TouchQueue_t msg;
    if (osMessageQueueGet(mid_MsgQueue, &msg, NULL, 0U) == osOK)
    {
        x = msg.x;
        y = msg.y;
        return true;
    }
    return false;
}

extern "C"
void StartTouchTask(void* argument)
{
    /*
     * Awaits StartMfxServiceTask to signal when a touch interrupt has been raised
     */
    TouchQueue_t msg = {0};
    static uint32_t xf; /* Final x value */
    static uint32_t yf; /* Final y value */
    int16_t  xc, yc;   /* Coordinates transfered to center screen */
    int16_t  xr, yr;   /* Coordinates after rotation */
    uint16_t x;        /* Initial x value */
    uint16_t y;        /* Initial y value */

    static uint8_t HwRotation = 0;

    /* Read firmware register to know if HW rotation is implemented */
    if (TS_IO_Read(STM32TouchController::TS_I2C_ADDRESS, FT3X67_FIRMID_REG) != 0x01)
    {
        HwRotation = 1;
    }

    while (1)
    {
        /* Waiting for MFX thread to signal Touch Interrupt has been raised */
        osSemaphoreAcquire(touchBinarySemHandle, osWaitForever);  // take the lock

        /* Reading touch coordinates from touchcontroller */
        if (ft3x67_ts_drv.DetectTouch(STM32TouchController::TS_I2C_ADDRESS))
        {
            ft3x67_ts_drv.GetXY(STM32TouchController::TS_I2C_ADDRESS, (uint16_t*)&x, (uint16_t*)&y);

            if (HwRotation == 0)
            {
                /* First translate coordonates to center screen */
                xc = x - 195;
                yc = y - 195;

                /* Apply rotation of 45 */
                xr = (int16_t)(SQRT_2 * (xc - yc) / 2);
                yr = (int16_t)(SQRT_2 * (xc + yc) / 2);

                /* Revert the initial translation */
                xf = xr + 195;
                yf = yr + 195;

                msg.x = xf;
                msg.y = yf;
            }
            else
            {
                msg.x = x;
                msg.y = y;
            }

            osMessageQueuePut(mid_MsgQueue, &msg, 0U, 0U);
        }
    }
}


/************************** LINK TS (TouchScreen) *****************************/
/**
  * @brief  Initializes Touchscreen low level.
  * @retval None
  */
void TS_IO_Init(void)
{
    /* Done in main */
}

/**
  * @brief  Writes a single data.
  * @param  Addr: I2C address
  * @param  Reg: Reg address
  * @param  Value: Data to be written
  * @retval None
  */
void TS_IO_Write(uint8_t Addr, uint8_t Reg, uint8_t Value)
{
    HAL_I2C_Mem_Write(&hi2c1, Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT, (uint8_t*)&Value, 1, I2C1_TIMEOUT_MAX);
}

/**
  * @brief  Reads a single data.
  * @param  Addr: I2C address
  * @param  Reg: Reg address
  * @retval Data to be read
  */
uint8_t TS_IO_Read(uint8_t Addr, uint8_t Reg)
{
    uint8_t value = 0x0;
    HAL_I2C_Mem_Read(&hi2c1, Addr, Reg, I2C_MEMADD_SIZE_8BIT, &value, 1, I2C1_TIMEOUT_MAX);
    return value;
}

/**
  * @brief  Reads multiple data with I2C communication
  *         channel from TouchScreen.
  * @param  Addr: I2C address
  * @param  Reg: Register address
  * @param  Buffer: Pointer to data buffer
  * @param  Length: Length of the data
  * @retval Number of read data
  */
uint16_t TS_IO_ReadMultiple(uint8_t Addr, uint8_t Reg, uint8_t* Buffer, uint16_t Length)
{
    return HAL_I2C_Mem_Read(&hi2c1, Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT, Buffer, Length, I2C1_TIMEOUT_MAX);
}

/**
  * @brief  Writes multiple data with I2C communication
  *         channel from MCU to TouchScreen.
  * @param  Addr: I2C address
  * @param  Reg: Register address
  * @param  Buffer: Pointer to data buffer
  * @param  Length: Length of the data
  * @retval None
  */
void TS_IO_WriteMultiple(uint8_t Addr, uint8_t Reg, uint8_t* Buffer, uint16_t Length)
{
    HAL_I2C_Mem_Write(&hi2c1, Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT, Buffer, Length, I2C1_TIMEOUT_MAX);
}

/* USER CODE END STM32TouchController */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
