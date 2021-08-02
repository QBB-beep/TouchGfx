/**
  ******************************************************************************
  * File Name          : TouchGFXHAL.cpp
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
#include <TouchGFXHAL.hpp>

/* USER CODE BEGIN TouchGFXHAL.cpp */

#include "stm32l4xx.h"

using namespace touchgfx;

#include <touchgfx/hal/OSWrappers.hpp>
#include <CortexMMCUInstrumentation.hpp>
#include <touchgfx/lcd/LCD.hpp>
#include <touchgfx/hal/GPIO.hpp>
#include <string.h>
#include <stdio.h>
#include "cmsis_os.h"

/*********************************************************************
*
*       Framebuffer dimension configuration
*/
#define BSP_LCD_IMAGE_WIDTH     (3072 / 2)
#define GFXMMU_LCD_FB_SIZE 245504
#define GFXMMU_LCD_FIRST 0
#define GFXMMU_LCD_LAST  389
#define GFXMMU_LCD_SIZE 390


#ifndef   GUI_NUM_FRAME_BUFFERS
#define GUI_NUM_FRAME_BUFFERS      1
#endif /* !GUI_NUM_FRAME_BUFFERS */
#ifndef   GUI_FRAME_BUFFER_WIDTH
#define GUI_FRAME_BUFFER_WIDTH   BSP_LCD_IMAGE_WIDTH
#endif /* !GUI_FRAME_BUFFER_WIDTH */
#ifndef   GUI_FRAME_BUFFER_HEIGHT
#define GUI_FRAME_BUFFER_HEIGHT  GFXMMU_LCD_SIZE
#endif /* !GUI_FRAME_BUFFER_HEIGHT */

/*********************************************************************
*
*       Framebuffer size configuration
*/
#ifndef   GUI_FRAME_BUFFER_SIZE
#define GUI_FRAME_BUFFER_SIZE      (GFXMMU_LCD_FB_SIZE * GUI_NUM_FRAME_BUFFERS)
#endif /* !GUI_FRAME_BUFFER_SIZE */

/**
* About this implementation:
* This class is for use ONLY with the DSI peripheral. If you have a regular RGB interface display, use the STM32F7HAL.cpp class instead.
*
* This implementation assumes that the DSI is configured to be in adapted command mode, with tearing effect set to external pin.
* Display will only be updated when there has actually been changes to the frame buffer.
*/
extern "C"
{
#include "stm32l4xx.h"
#include "stm32l4xx_hal_dsi.h"
#include "stm32l4xx_hal_ltdc.h"
#include "stm32l4xx_hal_gpio.h"

    extern DSI_HandleTypeDef  hdsi;
    extern LTDC_HandleTypeDef hltdc;

    /* Request tear interrupt at specific scanline. Implemented in BoardConfiguration.cpp */
    void LCD_ReqTear();

    /* Configures display to update indicated region of the screen (200pixel wide chunks) - 16bpp mode */
    void LCD_SetUpdateRegion(int idx);

    /* Configures display to update left half of the screen. Implemented in BoardConfiguration.cpp  - 24bpp mode*/
    void LCD_SetUpdateRegionLeft();

    /* Configures display to update right half of the screen. Implemented in BoardConfiguration.cpp - 24bpp mode*/
    void LCD_SetUpdateRegionRight();
}

volatile bool displayRefreshing = false;
volatile bool refreshRequested = true;
static bool doubleBufferingEnabled = false;
static uint16_t* currFbBase = 0;

static CortexMMCUInstrumentation mcuInstr;

TouchGFXHAL::TouchGFXHAL(touchgfx::DMA_Interface& dma, touchgfx::LCD& display, touchgfx::TouchController& tc, uint16_t width, uint16_t height)
/* USER CODE BEGIN TouchGFXHAL Constructor */
    : TouchGFXGeneratedHAL(dma,
                           display,
                           tc,
                           width, /* Align to match 832 pixel for optimal DSI transfer */
                           height)
      /* USER CODE END TouchGFXHAL Constructor */
{
    /* USER CODE BEGIN TouchGFXHAL Constructor Code */

    /* USER CODE END TouchGFXHAL Constructor Code */
}

void TouchGFXHAL::initialize()
{
    /* USER CODE BEGIN initialize step 1 */
    GPIO::init();
    /* USER CODE END initialize step 1 */

    // Calling parent implementation of initialize().
    //
    // To overwrite the generated implementation, omit call to parent function
    // and implemented needed functionality here.
    // Please note, HAL::initialize() must be called to initialize the framework.

    TouchGFXGeneratedHAL::initialize();

    /* USER CODE BEGIN initialize step 2 */
    /* Set display on */
    HAL_DSI_ShortWrite(&hdsi,
                       0,
                       DSI_DCS_SHORT_PKT_WRITE_P0,
                       DSI_SET_DISPLAY_ON,
                       0x0);

    mcuInstr.init();
    setMCUInstrumentation(&mcuInstr);
    enableMCULoadCalculation(true);
    /* USER CODE END initialize step 2 */
}

uint16_t* TouchGFXHAL::getTFTFrameBuffer() const
{
    return currFbBase;
}

void TouchGFXHAL::setTFTFrameBuffer(uint16_t* adr)
{
    if (doubleBufferingEnabled)
    {
        HAL_DSI_Stop(&hdsi);

        currFbBase = adr;

        /* Update LTDC layers base address */
        if (HAL_LTDC_SetAddress(&hltdc, (uint32_t)currFbBase, 0) != HAL_OK)
        {
            while (1);
        }
        __HAL_LTDC_LAYER_ENABLE(&hltdc, 0);
        __HAL_LTDC_RELOAD_IMMEDIATE_CONFIG(&hltdc);

        HAL_DSI_Start(&hdsi);
    }
}

/**
 * This function is called whenever the framework has performed a partial draw.
 *
 * @param rect The area of the screen that has been drawn, expressed in absolute coordinates.
 *
 * @see flushFrameBuffer().
 */
void TouchGFXHAL::flushFrameBuffer(const touchgfx::Rect& rect)
{
    // Calling parent implementation of flushFrameBuffer(const touchgfx::Rect& rect).
    //
    // To overwrite the generated implementation, omit call to parent function
    // and implemented needed functionality here.
    // Please note, HAL::flushFrameBuffer(const touchgfx::Rect& rect) must
    // be called to notify the touchgfx framework that flush has been performed.

    /* USER CODE BEGIN flushFrameBuffer step 1 */
    TouchGFXGeneratedHAL::flushFrameBuffer(rect);
    /* USER CODE END flushFrameBuffer step 1 */
}

void TouchGFXHAL::configureInterrupts()
{
    // These two priorities MUST be EQUAL, and MUST be functionally lower than RTOS scheduler interrupts.
    NVIC_SetPriority(DMA2D_IRQn, 7);
    NVIC_SetPriority(DSI_IRQn, 7);
}

/* Enable LCD line interrupt, when entering video (active) area */
void TouchGFXHAL::enableLCDControllerInterrupt()
{
    LCD_ReqTear();
    __HAL_DSI_CLEAR_FLAG(&hdsi, DSI_IT_ER);
    __HAL_DSI_CLEAR_FLAG(&hdsi, DSI_IT_TE);
    __HAL_DSI_ENABLE_IT(&hdsi, DSI_IT_TE);
    __HAL_DSI_ENABLE_IT(&hdsi, DSI_IT_ER);
}

void TouchGFXHAL::disableInterrupts()
{
    NVIC_DisableIRQ(DMA2D_IRQn);
    NVIC_DisableIRQ(DSI_IRQn);
}

void TouchGFXHAL::enableInterrupts()
{
    NVIC_EnableIRQ(DMA2D_IRQn);
    NVIC_EnableIRQ(DSI_IRQn);
    NVIC_EnableIRQ(LTDC_ER_IRQn);
}

void TouchGFXHAL::setFrameBufferStartAddresses(void* frameBuffer, void* doubleBuffer, void* animationStorage)
{
    bool useDoubleBuffering = (doubleBuffer != 0);
    bool useAnimationStorage = (animationStorage != 0);
    uint8_t* buffer = static_cast<uint8_t*>(frameBuffer);
    frameBuffer0 = reinterpret_cast<uint16_t*>(buffer);
    if (useDoubleBuffering)
    {
        buffer += GUI_FRAME_BUFFER_SIZE;
        frameBuffer1 = reinterpret_cast<uint16_t*>(buffer);
    }
    else
    {
        frameBuffer1 = 0;
    }
    if (useAnimationStorage)
    {
        buffer += GUI_FRAME_BUFFER_SIZE;
        frameBuffer2 = reinterpret_cast<uint16_t*>(buffer);
    }
    else
    {
        frameBuffer2 = 0;
    }
    USE_DOUBLE_BUFFERING  = useDoubleBuffering;
    USE_ANIMATION_STORAGE = useAnimationStorage;
    FRAME_BUFFER_WIDTH  = GUI_FRAME_BUFFER_WIDTH;
    FRAME_BUFFER_HEIGHT = GUI_FRAME_BUFFER_HEIGHT;

    // Make note of whether we are using double buffering.
    doubleBufferingEnabled = useDoubleBuffering;
    currFbBase = frameBuffer0;
}

bool TouchGFXHAL::beginFrame()
{
    refreshRequested = false;
    return touchgfx::HAL::beginFrame();
}

void TouchGFXHAL::endFrame()
{
    touchgfx::HAL::endFrame();
    if (frameBufferUpdatedThisFrame)
    {
        refreshRequested = true;
    }
}

extern "C" {
    void LCD_ReqTear(void)
    {
        static uint8_t ScanLineParams[2];

        uint16_t scanline = (GFXMMU_LCD_SIZE - 10);
        ScanLineParams[0] = scanline >> 8;
        ScanLineParams[1] = scanline & 0x00FF;

        HAL_DSI_LongWrite(&hdsi, 0, DSI_DCS_LONG_PKT_WRITE, 2, DSI_SET_TEAR_SCANLINE, ScanLineParams);
        // set_tear_on
        HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, DSI_SET_TEAR_ON, 0x00);
    }

    // Configure display to update specified region.
    void LCD_SetUpdateRegion(int idx)
    {
        // HAL_DSI_LongWrite(&hdsi, 0, DSI_DCS_LONG_PKT_WRITE, 4, DSI_SET_COLUMN_ADDRESS, pCols[idx]);
        uint8_t InitParam1[4] = {0x00, 0x04, 0x01, 0x89}; // MODIF OFe: adjusted w/ real image
        HAL_DSI_LongWrite(&hdsi, 0, DSI_DCS_LONG_PKT_WRITE, 4, DSI_SET_COLUMN_ADDRESS, InitParam1);
    }
    /**
      * @brief  Tearing Effect DSI callback.
      * @param  hdsi  pointer to a DSI_HandleTypeDef structure that contains
      *               the configuration information for the DSI.
      * @retval None
      */
    void HAL_DSI_TearingEffectCallback(DSI_HandleTypeDef* hdsi)
    {
        // Tearing effect interrupt. Occurs periodically (every 15.7 ms on 469 eval/disco boards)

        if (osKernelGetState() == osKernelRunning)
        {
            touchgfx::GPIO::set(touchgfx::GPIO::VSYNC_FREQ);
            touchgfx::HAL::getInstance()->vSync();
            touchgfx::OSWrappers::signalVSync();
            if (!doubleBufferingEnabled && touchgfx::HAL::getInstance())
            {
                // In single buffering, only require that the system waits for display update to be finished if we
                // actually intend to update the display in this frame.
                touchgfx::HAL::getInstance()->lockDMAToFrontPorch(refreshRequested);
            }
        }

        if (refreshRequested && !displayRefreshing)
        {
            if (osKernelGetState() == osKernelRunning)
            {
                // We have an update pending.
                if (doubleBufferingEnabled && touchgfx::HAL::getInstance())
                {
                    // Swap frame buffers immediately instead of waiting for the task to be scheduled in.
                    // Note: task will also swap when it wakes up, but that operation is guarded and will not have
                    // any effect if already swapped.
                    touchgfx::HAL::getInstance()->swapFrameBuffers();
                }
                displayRefreshing = true;
            }
            //Set update whole display region.
            LCD_SetUpdateRegion(0);

            // Transfer a quarter screen of pixel data.
            HAL_DSI_Refresh(hdsi);
        }
        else
        {
            if (osKernelGetState() == osKernelRunning)
            {
                touchgfx::GPIO::clear(touchgfx::GPIO::VSYNC_FREQ);
            }
        }
    }

    /**
      * @brief  End of Refresh DSI callback.
      * @param  hdsi  pointer to a DSI_HandleTypeDef structure that contains
      *               the configuration information for the DSI.
      * @retval None
      */
    void HAL_DSI_EndOfRefreshCallback(DSI_HandleTypeDef* hdsi)
    {
        // End-of-refresh interrupt. Meaning one of the 4 regions have been transferred.

        // HAL_DSI_Stop(hdsi);
        //
        // LTDC_LAYER(hltdc, 0)->CFBAR = (uint32_t)currFbBase;
        // LTDC->SRCR = (uint32_t)LTDC_SRCR_IMR;
        // LCD_SetUpdateRegion(0);
        // DSI->WCR |= DSI_WCR_DSIEN;
        //
        // HAL_DSI_Start(hdsi);

        if (osKernelGetState() == osKernelRunning)
        {
            touchgfx::GPIO::clear(touchgfx::GPIO::VSYNC_FREQ);

            displayRefreshing = false;
            if (touchgfx::HAL::getInstance())
            {
                // Signal to the framework that display update has finished.
                touchgfx::HAL::getInstance()->frontPorchEntered();
            }
        }
    }

    portBASE_TYPE IdleTaskHook(void* p)
    {
        if ((int)p) //idle task sched out
        {
            touchgfx::HAL::getInstance()->setMCUActive(true);
        }
        else //idle task sched in
        {
            touchgfx::HAL::getInstance()->setMCUActive(false);
        }
        return pdTRUE;
    }
}

/* USER CODE END TouchGFXHAL.cpp */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
