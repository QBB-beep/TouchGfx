/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "gfxmmu_lut.h"
#include "cmsis_os.h"
#include "app_touchgfx.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "gfxmmu_lut_touchgfx.h"
#include "stm32l4r9i_discovery_io.h"
#include "mx25lm51245g/mx25lm51245g.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define BRIGHTNESS_NORMAL 200

/* OSPI Error codes */
#define OSPI_NOR_OK            ((uint8_t)0x00)
#define OSPI_NOR_ERROR         ((uint8_t)0x01)
#define OSPI_NOR_BUSY          ((uint8_t)0x02)
#define OSPI_NOR_NOT_SUPPORTED ((uint8_t)0x04)
#define OSPI_NOR_SUSPENDED     ((uint8_t)0x08)

#define OSPI_NOR_OCTAL_DISABLE 0x0
#define OSPI_NOR_OCTAL_ENABLE 0x1

#define DUMMY_CYCLES_READ_OCTAL MX25LM51245G_DUMMY_CYCLES_READ_OCTAL_66M
#define DUMMY_CYCLES_CR_CFG MX25LM51245G_CR2_DC_66M
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CRC_HandleTypeDef hcrc;

DMA2D_HandleTypeDef hdma2d;

DSI_HandleTypeDef hdsi;

GFXMMU_HandleTypeDef hgfxmmu;

I2C_HandleTypeDef hi2c1;

LTDC_HandleTypeDef hltdc;

OSPI_HandleTypeDef hospi1;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for guiTask */
osThreadId_t guiTaskHandle;
const osThreadAttr_t guiTask_attributes = {
  .name = "guiTask",
  .stack_size = 4096 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for touchTask */
osThreadId_t touchTaskHandle;
const osThreadAttr_t touchTask_attributes = {
  .name = "touchTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for mfxServiceTask */
osThreadId_t mfxServiceTaskHandle;
const osThreadAttr_t mfxServiceTask_attributes = {
  .name = "mfxServiceTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for mfxBinarySem */
osSemaphoreId_t mfxBinarySemHandle;
const osSemaphoreAttr_t mfxBinarySem_attributes = {
  .name = "mfxBinarySem"
};
/* Definitions for touchBinarySem */
osSemaphoreId_t touchBinarySemHandle;
const osSemaphoreAttr_t touchBinarySem_attributes = {
  .name = "touchBinarySem"
};
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CRC_Init(void);
static void MX_GFXMMU_Init(void);
static void MX_LTDC_Init(void);
static void MX_DMA2D_Init(void);
static void MX_I2C1_Init(void);
static void MX_DSIHOST_DSI_Init(void);
static void MX_OCTOSPI1_Init(void);
void StartDefaultTask(void *argument);
void TouchGFX_Task(void *argument);
extern void StartTouchTask(void *argument);
void StartMfxServiceTask(void *argument);

/* USER CODE BEGIN PFP */
static void LCD_PowerOn(void);
static void LCD_PowerOff(void);
static uint8_t ResetMemory(OSPI_HandleTypeDef *hospi);
static uint8_t OctalMode(OSPI_HandleTypeDef *hospi, uint8_t Operation);
static uint8_t AutoPollingMemReady(OSPI_HandleTypeDef *hospi, uint32_t Timeout);
static uint8_t WriteEnable(OSPI_HandleTypeDef *hospi);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_CRC_Init();
  MX_GFXMMU_Init();
  MX_LTDC_Init();
  MX_DMA2D_Init();
  MX_I2C1_Init();
  MX_DSIHOST_DSI_Init();
  MX_OCTOSPI1_Init();
  MX_TouchGFX_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */

  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of mfxBinarySem */
  mfxBinarySemHandle = osSemaphoreNew(1, 1, &mfxBinarySem_attributes);

  /* creation of touchBinarySem */
  touchBinarySemHandle = osSemaphoreNew(1, 1, &touchBinarySem_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  osSemaphoreAcquire(mfxBinarySemHandle, 0); // take the lock
  osSemaphoreAcquire(touchBinarySemHandle, 0); // take the lock
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */

  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of guiTask */
  guiTaskHandle = osThreadNew(TouchGFX_Task, NULL, &guiTask_attributes);

  /* creation of touchTask */
  touchTaskHandle = osThreadNew(StartTouchTask, NULL, &touchTask_attributes);

  /* creation of mfxServiceTask */
  mfxServiceTaskHandle = osThreadNew(StartMfxServiceTask, NULL, &mfxServiceTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 60;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1|RCC_PERIPHCLK_OSPI
                              |RCC_PERIPHCLK_DSI|RCC_PERIPHCLK_LTDC;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  PeriphClkInit.DsiClockSelection = RCC_DSICLKSOURCE_DSIPHY;
  PeriphClkInit.OspiClockSelection = RCC_OSPICLKSOURCE_SYSCLK;
  PeriphClkInit.LtdcClockSelection = RCC_LTDCCLKSOURCE_PLLSAI2_DIV4;
  PeriphClkInit.PLLSAI2.PLLSAI2Source = RCC_PLLSOURCE_MSI;
  PeriphClkInit.PLLSAI2.PLLSAI2M = 1;
  PeriphClkInit.PLLSAI2.PLLSAI2N = 80;
  PeriphClkInit.PLLSAI2.PLLSAI2P = RCC_PLLP_DIV2;
  PeriphClkInit.PLLSAI2.PLLSAI2R = RCC_PLLR_DIV4;
  PeriphClkInit.PLLSAI2.PLLSAI2Q = RCC_PLLQ_DIV2;
  PeriphClkInit.PLLSAI2.PLLSAI2ClockOut = RCC_PLLSAI2_LTDCCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

/**
  * @brief DMA2D Initialization Function
  * @param None
  * @retval None
  */
static void MX_DMA2D_Init(void)
{

  /* USER CODE BEGIN DMA2D_Init 0 */

  /* USER CODE END DMA2D_Init 0 */

  /* USER CODE BEGIN DMA2D_Init 1 */

  /* USER CODE END DMA2D_Init 1 */
  hdma2d.Instance = DMA2D;
  hdma2d.Init.Mode = DMA2D_M2M;
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB565;
  hdma2d.Init.OutputOffset = 0;
  hdma2d.Init.BytesSwap = DMA2D_BYTES_REGULAR;
  hdma2d.Init.LineOffsetMode = DMA2D_LOM_PIXELS;
  hdma2d.LayerCfg[1].InputOffset = 0;
  hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB565;
  hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d.LayerCfg[1].InputAlpha = 0;
  hdma2d.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA;
  hdma2d.LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR;
  if (HAL_DMA2D_Init(&hdma2d) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DMA2D_Init 2 */

  /* USER CODE END DMA2D_Init 2 */

}

/**
  * @brief DSIHOST Initialization Function
  * @param None
  * @retval None
  */
static void MX_DSIHOST_DSI_Init(void)
{

  /* USER CODE BEGIN DSIHOST_Init 0 */
  uint8_t InitParam1[4]= {0x00, 0x04, 0x01, 0x89};
  uint8_t InitParam2[4]= {0x00, 0x00, 0x01, 0x85};
  /* USER CODE END DSIHOST_Init 0 */

  DSI_PLLInitTypeDef PLLInit = {0};
  DSI_HOST_TimeoutTypeDef HostTimeouts = {0};
  DSI_PHY_TimerTypeDef PhyTimings = {0};
  DSI_LPCmdTypeDef LPCmd = {0};
  DSI_CmdCfgTypeDef CmdCfg = {0};

  /* USER CODE BEGIN DSIHOST_Init 1 */
  hdsi.Instance = DSI;
  __HAL_DSI_RESET_HANDLE_STATE(&hdsi);
  /* Power off then on on LCD */
  LCD_PowerOff();
  LCD_PowerOn();
  /* USER CODE END DSIHOST_Init 1 */
  hdsi.Instance = DSI;
  hdsi.Init.AutomaticClockLaneControl = DSI_AUTO_CLK_LANE_CTRL_DISABLE;
  hdsi.Init.TXEscapeCkdiv = 4;
  hdsi.Init.NumberOfLanes = DSI_ONE_DATA_LANE;
  PLLInit.PLLNDIV = 125;
  PLLInit.PLLIDF = DSI_PLL_IN_DIV4;
  PLLInit.PLLODF = DSI_PLL_OUT_DIV1;
  if (HAL_DSI_Init(&hdsi, &PLLInit) != HAL_OK)
  {
    Error_Handler();
  }
  HostTimeouts.TimeoutCkdiv = 1;
  HostTimeouts.HighSpeedTransmissionTimeout = 0;
  HostTimeouts.LowPowerReceptionTimeout = 0;
  HostTimeouts.HighSpeedReadTimeout = 0;
  HostTimeouts.LowPowerReadTimeout = 0;
  HostTimeouts.HighSpeedWriteTimeout = 0;
  HostTimeouts.HighSpeedWritePrespMode = DSI_HS_PM_DISABLE;
  HostTimeouts.LowPowerWriteTimeout = 0;
  HostTimeouts.BTATimeout = 0;
  if (HAL_DSI_ConfigHostTimeouts(&hdsi, &HostTimeouts) != HAL_OK)
  {
    Error_Handler();
  }
  PhyTimings.ClockLaneHS2LPTime = 28;
  PhyTimings.ClockLaneLP2HSTime = 33;
  PhyTimings.DataLaneHS2LPTime = 15;
  PhyTimings.DataLaneLP2HSTime = 25;
  PhyTimings.DataLaneMaxReadTime = 0;
  PhyTimings.StopWaitTime = 7;
  if (HAL_DSI_ConfigPhyTimer(&hdsi, &PhyTimings) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DSI_ConfigFlowControl(&hdsi, DSI_FLOW_CONTROL_BTA) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DSI_SetLowPowerRXFilter(&hdsi, 10000) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DSI_ConfigErrorMonitor(&hdsi, HAL_DSI_ERROR_NONE) != HAL_OK)
  {
    Error_Handler();
  }
  LPCmd.LPGenShortWriteNoP = DSI_LP_GSW0P_ENABLE;
  LPCmd.LPGenShortWriteOneP = DSI_LP_GSW1P_ENABLE;
  LPCmd.LPGenShortWriteTwoP = DSI_LP_GSW2P_ENABLE;
  LPCmd.LPGenShortReadNoP = DSI_LP_GSR0P_ENABLE;
  LPCmd.LPGenShortReadOneP = DSI_LP_GSR1P_ENABLE;
  LPCmd.LPGenShortReadTwoP = DSI_LP_GSR2P_ENABLE;
  LPCmd.LPGenLongWrite = DSI_LP_GLW_DISABLE;
  LPCmd.LPDcsShortWriteNoP = DSI_LP_DSW0P_ENABLE;
  LPCmd.LPDcsShortWriteOneP = DSI_LP_DSW1P_ENABLE;
  LPCmd.LPDcsShortReadNoP = DSI_LP_DSR0P_ENABLE;
  LPCmd.LPDcsLongWrite = DSI_LP_DLW_DISABLE;
  LPCmd.LPMaxReadPacket = DSI_LP_MRDP_DISABLE;
  LPCmd.AcknowledgeRequest = DSI_ACKNOWLEDGE_DISABLE;
  if (HAL_DSI_ConfigCommand(&hdsi, &LPCmd) != HAL_OK)
  {
    Error_Handler();
  }
  CmdCfg.VirtualChannelID = 0;
  CmdCfg.ColorCoding = DSI_RGB888;
  CmdCfg.CommandSize = 390;
  CmdCfg.TearingEffectSource = DSI_TE_EXTERNAL;
  CmdCfg.TearingEffectPolarity = DSI_TE_FALLING_EDGE;
  CmdCfg.HSPolarity = DSI_HSYNC_ACTIVE_LOW;
  CmdCfg.VSPolarity = DSI_VSYNC_ACTIVE_LOW;
  CmdCfg.DEPolarity = DSI_DATA_ENABLE_ACTIVE_HIGH;
  CmdCfg.VSyncPol = DSI_VSYNC_FALLING;
  CmdCfg.AutomaticRefresh = DSI_AR_DISABLE;
  CmdCfg.TEAcknowledgeRequest = DSI_TE_ACKNOWLEDGE_ENABLE;
  if (HAL_DSI_ConfigAdaptedCommandMode(&hdsi, &CmdCfg) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DSI_SetGenericVCID(&hdsi, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DSIHOST_Init 2 */
    /* Enable DSI */
    __HAL_DSI_ENABLE(&hdsi);

  /* LCD Power ON sequence */
  /* Step 1 */
  /* Go to command 2 */
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0xFE, 0x01);
  /* IC Frame rate control, set power, sw mapping, mux swithc timing command */
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x06, 0x62);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x0E, 0x80);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x0F, 0x80);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x10, 0x71);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x13, 0x81);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x14, 0x81);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x15, 0x82);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x16, 0x82);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x18, 0x88);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x19, 0x55);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x1A, 0x10);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x1C, 0x99);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x1D, 0x03);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x1E, 0x03);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x1F, 0x03);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x20, 0x03);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x25, 0x03);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x26, 0x8D);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x2A, 0x03);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x2B, 0x8D);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x36, 0x00);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x37, 0x10);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x3A, 0x00);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x3B, 0x00);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x3D, 0x20);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x3F, 0x3A);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x40, 0x30);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x41, 0x1A);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x42, 0x33);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x43, 0x22);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x44, 0x11);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x45, 0x66);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x46, 0x55);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x47, 0x44);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x4C, 0x33);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x4D, 0x22);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x4E, 0x11);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x4F, 0x66);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x50, 0x55);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x51, 0x44);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x57, 0x33);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x6B, 0x1B);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x70, 0x55);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x74, 0x0C);

  /* Go to command 3 */
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0xFE, 0x02);
  /* Set the VGMP/VGSP coltage control */
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x9B, 0x40);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x9C, 0x00);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x9D, 0x20);

  /* Go to command 4 */
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0xFE, 0x03);
  /* Set the VGMP/VGSP coltage control */
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x9B, 0x40);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x9C, 0x00);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x9D, 0x20);

  /* Go to command 5 */
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0xFE, 0x04);
  /* VSR command */
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x5D, 0x10);
  /* VSR1 timing set */
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x00, 0x8D);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x01, 0x00);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x02, 0x01);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x03, 0x01);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x04, 0x10);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x05, 0x01);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x06, 0xA7);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x07, 0x20);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x08, 0x00);
  /* VSR2 timing set */
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x09, 0xC2);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x0A, 0x00);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x0B, 0x02);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x0C, 0x01);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x0D, 0x40);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x0E, 0x06);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x0F, 0x01);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x10, 0xA7);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x11, 0x00);
  /* VSR3 timing set */
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x12, 0xC2);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x13, 0x00);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x14, 0x02);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x15, 0x01);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x16, 0x40);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x17, 0x07);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x18, 0x01);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x19, 0xA7);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x1A, 0x00);
  /* VSR4 timing set */
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x1B, 0x82);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x1C, 0x00);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x1D, 0xFF);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x1E, 0x05);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x1F, 0x60);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x20, 0x02);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x21, 0x01);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x22, 0x7C);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x23, 0x00);
  /* VSR5 timing set */
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x24, 0xC2);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x25, 0x00);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x26, 0x04);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x27, 0x02);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x28, 0x70);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x29, 0x05);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x2A, 0x74);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x2B, 0x8D);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x2D, 0x00);
  /* VSR6 timing set */
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x2F, 0xC2);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x30, 0x00);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x31, 0x04);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x32, 0x02);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x33, 0x70);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x34, 0x07);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x35, 0x74);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x36, 0x8D);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x37, 0x00);
  /* VSR marping command */
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x5E, 0x20);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x5F, 0x31);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x60, 0x54);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x61, 0x76);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x62, 0x98); /* ï¿?0x62ï¿? is suggested instead of ï¿?0x52ï¿? */

  /* Go to command 6 */
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0xFE, 0x05);
  /* Set the ELVSS voltage */
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x05, 0x17); /* ï¿?0x17ï¿? is suggested instead of ï¿?0x07ï¿? for SM3321 */
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x2A, 0x04);
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x91, 0x00);

  /* Go back in standard commands */
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0xFE, 0x00);

  /* Set tear off */
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, DSI_SET_TEAR_OFF, 0x0);

  /* Set DSI mode to internal timing added vs ORIGINAL for Command mode */
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0xC2, 0x0);

  /* Set memory address MODIFIED vs ORIGINAL */
  HAL_DSI_LongWrite(&hdsi, 0, DSI_DCS_LONG_PKT_WRITE, 4, DSI_SET_COLUMN_ADDRESS, InitParam1);
  HAL_DSI_LongWrite(&hdsi, 0, DSI_DCS_LONG_PKT_WRITE, 4, DSI_SET_PAGE_ADDRESS, InitParam2);

  /* Sleep out */
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P0, DSI_EXIT_SLEEP_MODE, 0x0);

  HAL_Delay(120);

  /* Set default Brightness */
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x51, BRIGHTNESS_NORMAL);

  /* Set display off */
  if(HAL_DSI_ShortWrite(&hdsi,
                        0,
                        DSI_DCS_SHORT_PKT_WRITE_P0,
                        DSI_SET_DISPLAY_OFF,
                        0x0) != HAL_OK)
  {
    Error_Handler();
  }

  /* Enable DSI Wrapper */
  __HAL_DSI_WRAPPER_ENABLE(&hdsi);
  /* USER CODE END DSIHOST_Init 2 */

}

/**
  * @brief GFXMMU Initialization Function
  * @param None
  * @retval None
  */
static void MX_GFXMMU_Init(void)
{

  /* USER CODE BEGIN GFXMMU_Init 0 */

  /* USER CODE END GFXMMU_Init 0 */

  /* USER CODE BEGIN GFXMMU_Init 1 */
  hgfxmmu.Instance = GFXMMU;
  __HAL_GFXMMU_RESET_HANDLE_STATE(&hgfxmmu);
  /* USER CODE END GFXMMU_Init 1 */
  hgfxmmu.Instance = GFXMMU;
  hgfxmmu.Init.BlocksPerLine = GFXMMU_192BLOCKS;
  hgfxmmu.Init.DefaultValue = 0x00000000;
  hgfxmmu.Init.Buffers.Buf0Address = 0x20040000;
  hgfxmmu.Init.Buffers.Buf1Address = 0;
  hgfxmmu.Init.Buffers.Buf2Address = 0;
  hgfxmmu.Init.Buffers.Buf3Address = 0;
  hgfxmmu.Init.Interrupts.Activation = DISABLE;
  if (HAL_GFXMMU_Init(&hgfxmmu) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_GFXMMU_ConfigLut(&hgfxmmu, GFXMMU_LUT_FIRST, GFXMMU_LUT_SIZE, (uint32_t)gfxmmu_lut_config) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN GFXMMU_Init 2 */
  if (HAL_GFXMMU_ConfigLut(&hgfxmmu, GFXMMU_LUT_FIRST_TOUCHGFX, GFXMMU_LUT_SIZE_TOUCHGFX, (uint32_t)gfxmmu_lut_config_touchgfx) != HAL_OK)
  {
    Error_Handler();
  }
  /* Disable non visible lines : from line 390 to 1023 */
  if (HAL_GFXMMU_DisableLutLines(&hgfxmmu, 390, 634) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE END GFXMMU_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x307075B1;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */
  /* Initialize mfxstm32l152 GPIO expander driver */
  mfxstm32l152_io_drv.Init(IO_I2C_ADDRESS);
  mfxstm32l152_io_drv.Start(IO_I2C_ADDRESS, IO_PIN_ALL);

  /* Configure touchscreen interrupt pin */
  mfxstm32l152_io_drv.Config(IO_I2C_ADDRESS, TS_INT_PIN, IO_MODE_IT_FALLING_EDGE_PU);

  /* Configure IO Expander interrupt */
  MFX_IO_ITConfig();
  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief LTDC Initialization Function
  * @param None
  * @retval None
  */
static void MX_LTDC_Init(void)
{

  /* USER CODE BEGIN LTDC_Init 0 */

  /* USER CODE END LTDC_Init 0 */

  LTDC_LayerCfgTypeDef pLayerCfg = {0};

  /* USER CODE BEGIN LTDC_Init 1 */
  HAL_LTDC_DeInit(&hltdc);
  hltdc.Instance = LTDC;
  __HAL_LTDC_RESET_HANDLE_STATE(&hltdc);
  /* USER CODE END LTDC_Init 1 */
  hltdc.Instance = LTDC;
  hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
  hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
  hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
  hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
  hltdc.Init.HorizontalSync = 0;
  hltdc.Init.VerticalSync = 0;
  hltdc.Init.AccumulatedHBP = 1;
  hltdc.Init.AccumulatedVBP = 1;
  hltdc.Init.AccumulatedActiveW = 391;
  hltdc.Init.AccumulatedActiveH = 391;
  hltdc.Init.TotalWidth = 392;
  hltdc.Init.TotalHeigh = 392;
  hltdc.Init.Backcolor.Blue = 0;
  hltdc.Init.Backcolor.Green = 0;
  hltdc.Init.Backcolor.Red = 0;
  if (HAL_LTDC_Init(&hltdc) != HAL_OK)
  {
    Error_Handler();
  }
  pLayerCfg.WindowX0 = 0;
  pLayerCfg.WindowX1 = 390;
  pLayerCfg.WindowY0 = 0;
  pLayerCfg.WindowY1 = 390;
  pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
  pLayerCfg.Alpha = 0xFF;
  pLayerCfg.Alpha0 = 0;
  pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
  pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
  pLayerCfg.FBStartAdress = GFXMMU_VIRTUAL_BUFFER0_BASE;
  pLayerCfg.ImageWidth = 1536;
  pLayerCfg.ImageHeight = 390;
  pLayerCfg.Backcolor.Blue = 0;
  pLayerCfg.Backcolor.Green = 0;
  pLayerCfg.Backcolor.Red = 0;
  if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_LTDC_SetPitch(&hltdc, 1536, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LTDC_Init 2 */
  __HAL_LTDC_LAYER_ENABLE(&hltdc, 0);
  /* USER CODE END LTDC_Init 2 */

}

/**
  * @brief OCTOSPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_OCTOSPI1_Init(void)
{

  /* USER CODE BEGIN OCTOSPI1_Init 0 */

  /* USER CODE END OCTOSPI1_Init 0 */

  OSPIM_CfgTypeDef OSPIM_Cfg_Struct = {0};

  /* USER CODE BEGIN OCTOSPI1_Init 1 */
  hospi1.Instance = OCTOSPI1;
  if (HAL_OSPI_DeInit(&hospi1) != HAL_OK)
  {
    Error_Handler(); //OSPI_NOR_ERROR;
  }
  /* USER CODE END OCTOSPI1_Init 1 */
  /* OCTOSPI1 parameter configuration*/
  hospi1.Instance = OCTOSPI1;
  hospi1.Init.FifoThreshold = 4;
  hospi1.Init.DualQuad = HAL_OSPI_DUALQUAD_DISABLE;
  hospi1.Init.MemoryType = HAL_OSPI_MEMTYPE_MACRONIX;
  hospi1.Init.DeviceSize = 26;
  hospi1.Init.ChipSelectHighTime = 2;
  hospi1.Init.FreeRunningClock = HAL_OSPI_FREERUNCLK_DISABLE;
  hospi1.Init.ClockMode = HAL_OSPI_CLOCK_MODE_0;
  hospi1.Init.ClockPrescaler = 2;
  hospi1.Init.SampleShifting = HAL_OSPI_SAMPLE_SHIFTING_NONE;
  hospi1.Init.DelayHoldQuarterCycle = HAL_OSPI_DHQC_ENABLE;
  hospi1.Init.ChipSelectBoundary = 0;
  hospi1.Init.DelayBlockBypass = HAL_OSPI_DELAY_BLOCK_USED;
  if (HAL_OSPI_Init(&hospi1) != HAL_OK)
  {
    Error_Handler();
  }
  OSPIM_Cfg_Struct.ClkPort = 2;
  OSPIM_Cfg_Struct.DQSPort = 2;
  OSPIM_Cfg_Struct.NCSPort = 2;
  OSPIM_Cfg_Struct.IOLowPort = HAL_OSPIM_IOPORT_2_LOW;
  OSPIM_Cfg_Struct.IOHighPort = HAL_OSPIM_IOPORT_2_HIGH;
  if (HAL_OSPIM_Config(&hospi1, &OSPIM_Cfg_Struct, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN OCTOSPI1_Init 2 */
  /* OctoSPI memory reset */
  if (ResetMemory(&hospi1) != OSPI_NOR_OK)
  {
    Error_Handler();
  }

  /* Insert delay */
  HAL_Delay(1);

  /* OctoSPI octal enable */
  if (OctalMode(&hospi1, OSPI_NOR_OCTAL_ENABLE) != OSPI_NOR_OK)
  {
    Error_Handler();
  }

  OSPI_RegularCmdTypeDef   sCommand;
  OSPI_MemoryMappedTypeDef sMemMappedCfg;

  /* Enable write operations */
  if (WriteEnable(&hospi1) != OSPI_NOR_OK)
  {
    Error_Handler(); // OSPI_NOR_ERROR;
  }

  /* Configure the command for the read instruction */
  sCommand.OperationType         = HAL_OSPI_OPTYPE_READ_CFG;
  sCommand.FlashId               = HAL_OSPI_FLASH_ID_1;
  sCommand.InstructionMode       = HAL_OSPI_INSTRUCTION_8_LINES;
  sCommand.InstructionSize       = HAL_OSPI_INSTRUCTION_16_BITS;
  sCommand.AddressMode           = HAL_OSPI_ADDRESS_8_LINES;
  sCommand.AddressSize           = HAL_OSPI_ADDRESS_32_BITS;
  sCommand.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode              = HAL_OSPI_DATA_8_LINES;
  sCommand.DummyCycles           = DUMMY_CYCLES_READ_OCTAL;
  sCommand.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;
  sCommand.Instruction           = OCTAL_IO_DTR_READ_CMD;
  sCommand.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_ENABLE;
  sCommand.AddressDtrMode        = HAL_OSPI_ADDRESS_DTR_ENABLE;
  sCommand.DataDtrMode           = HAL_OSPI_DATA_DTR_ENABLE;
  sCommand.DQSMode               = HAL_OSPI_DQS_ENABLE;

  if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler(); // OSPI_NOR_ERROR;
  }

  /* Configure the command for the write instruction */
  sCommand.OperationType         = HAL_OSPI_OPTYPE_WRITE_CFG;
  sCommand.Instruction           = OCTAL_PAGE_PROG_CMD;
  sCommand.DummyCycles           = 0;

  if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler(); // OSPI_NOR_ERROR;
  }

   /* Configure the memory mapped mode */
  sMemMappedCfg.TimeOutActivation = HAL_OSPI_TIMEOUT_COUNTER_DISABLE;

  if (HAL_OSPI_MemoryMapped(&hospi1, &sMemMappedCfg) != HAL_OK)
  {
    Error_Handler(); // OSPI_NOR_ERROR;
  }
  /* USER CODE END OCTOSPI1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOI_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  HAL_PWREx_EnableVddIO2();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(VSYNC_FREQ_GPIO_Port, VSYNC_FREQ_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : MFX_IRQ_Pin */
  GPIO_InitStruct.Pin = MFX_IRQ_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(MFX_IRQ_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : VSYNC_FREQ_Pin */
  GPIO_InitStruct.Pin = VSYNC_FREQ_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(VSYNC_FREQ_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

}

/* USER CODE BEGIN 4 */

/**
  * @brief  LCD power off
  *         Power off LCD.
  */
static void LCD_PowerOff(void)
{
  BSP_IO_Init();

  /* Activate DSI_RESET */
  BSP_IO_WritePin(IO_PIN_10, GPIO_PIN_RESET);

  /* Wait at least 5 ms */
  HAL_Delay(5);

  /* Set DSI_POWER_ON to analog mode only if psram is not currently used */
  /* Disable first DSI_1V8_PWRON then DSI_3V3_PWRON */
  BSP_IO_ConfigPin(AGPIO_PIN_2, IO_MODE_ANALOG);
  BSP_IO_ConfigPin(IO_PIN_8, IO_MODE_ANALOG);
}

/**
  * @brief  LCD power on
  *         Power on LCD.
  */
static void LCD_PowerOn(void)
{
  BSP_IO_Init();

  /* Configure the GPIO connected to DSI_3V3_POWERON signal as output low */
  /* to activate 3V3_LCD. VDD_LCD is also activated if VDD = 3,3V */
  BSP_IO_WritePin(IO_PIN_8, GPIO_PIN_SET);
  BSP_IO_ConfigPin(IO_PIN_8, IO_MODE_OUTPUT);
  BSP_IO_WritePin(IO_PIN_8, GPIO_PIN_RESET);

  /* Wait at least 1ms before enabling 1V8_LCD */
  HAL_Delay(1);

  /* Configure the GPIO connected to DSI_1V8_POWERON signal as output low */
  /* to activate 1V8_LCD. VDD_LCD is also activated if VDD = 1,8V */
  BSP_IO_WritePin(AGPIO_PIN_2, GPIO_PIN_SET);
  BSP_IO_ConfigPin(AGPIO_PIN_2, IO_MODE_OUTPUT);
  BSP_IO_WritePin(AGPIO_PIN_2, GPIO_PIN_RESET);

  /* Wait at least 15 ms (minimum reset low width is 10ms and add margin for 1V8_LCD ramp-up) */
  HAL_Delay(15);

  /* Configure the GPIO connected to DSI_RESET signal */
  BSP_IO_ConfigPin(IO_PIN_10, IO_MODE_OUTPUT);

  /* Desactivate DSI_RESET */
  BSP_IO_WritePin(IO_PIN_10, GPIO_PIN_SET);

  /* Wait reset complete time (maximum time is 5ms when LCD in sleep mode and 120ms when LCD is not in sleep mode) */
  HAL_Delay(120);
}

/********************************* LINK MFX ***********************************/
/**
  * @brief  Initializes MFX low level.
  * @retval None
  */
void MFX_IO_Init(void)
{
  /* Wait for device ready */

  if(HAL_I2C_IsDeviceReady(&hi2c1, IO_I2C_ADDRESS, 4, 50) != HAL_OK)
  {
     Error_Handler();
  }
}

/**
  * @brief  Configures MFX low level interrupt.
  * @retval None
  */
void MFX_IO_ITConfig(void)
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  /* Enable the GPIO clock */
  MFX_INT_GPIO_CLK_ENABLE();

  /* MFX_OUT_IRQ (normally used for EXTI_WKUP) */
  GPIO_InitStruct.Pin   = MFX_INT_PIN;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Mode  = GPIO_MODE_IT_RISING;
  HAL_GPIO_Init(MFX_INT_GPIO_PORT, &GPIO_InitStruct);

  /* Enable and set GPIO EXTI Interrupt to the lowest priority */
  HAL_NVIC_SetPriority((IRQn_Type)(MFX_INT_EXTI_IRQn), 0x0F, 0x0F);
  HAL_NVIC_EnableIRQ((IRQn_Type)(MFX_INT_EXTI_IRQn));
}

/**
  * @brief  Configures MFX wke up  pin.
  * @retval None
  */
void MFX_IO_EnableWakeupPin(void)
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  /* Enable wakeup gpio clock */
  MFX_WAKEUP_GPIO_CLK_ENABLE();

  /* MFX wakeup pin configuration */
  GPIO_InitStruct.Pin   = MFX_WAKEUP_PIN;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  HAL_GPIO_Init(MFX_WAKEUP_GPIO_PORT, &GPIO_InitStruct);
}

/**
  * @brief  Wakeup MFX.
  * @retval None
  */
void MFX_IO_Wakeup(void)
{
  /* Set Wakeup pin to high to wakeup Idd measurement component from standby mode */
  HAL_GPIO_WritePin(MFX_WAKEUP_GPIO_PORT, MFX_WAKEUP_PIN, GPIO_PIN_SET);

  /* Wait */
  HAL_Delay(1);

  /* Set gpio pin basck to low */
  HAL_GPIO_WritePin(MFX_WAKEUP_GPIO_PORT, MFX_WAKEUP_PIN, GPIO_PIN_RESET);
}

/**
  * @brief  MFX writes single data.
  * @param  Addr: I2C address
  * @param  Reg: Register address
  * @param  Value: Data to be written
  * @retval None
  */
void MFX_IO_Write(uint16_t Addr, uint8_t Reg, uint8_t Value)
{
//  I2C1_WriteData(Addr, Reg, I2C_MEMADD_SIZE_8BIT, Value);
  HAL_I2C_Mem_Write(&hi2c1, Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT, &Value, 1, DISCOVERY_I2C1_TIMEOUT_MAX);
}

/**
  * @brief  MFX reads single data.
  * @param  Addr: I2C address
  * @param  Reg: Register address
  * @retval Read data
  */
uint8_t MFX_IO_Read(uint16_t Addr, uint8_t Reg)
{
//  return I2C1_ReadData(Addr, Reg, I2C_MEMADD_SIZE_8BIT);
  uint8_t value = 0x00;
  HAL_I2C_Mem_Read(&hi2c1, Addr, Reg, I2C_MEMADD_SIZE_8BIT, &value, 1, DISCOVERY_I2C1_TIMEOUT_MAX);
  return value;
}

/**
  * @brief  MFX reads multiple data.
  * @param  Addr: I2C address
  * @param  Reg: Register address
  * @param  Buffer: Pointer to data buffer
  * @param  Length: Length of the data
  * @retval Number of read data
  */
uint16_t MFX_IO_ReadMultiple(uint16_t Addr, uint8_t Reg, uint8_t *Buffer, uint16_t Length)
{
// return I2C1_ReadBuffer(Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT, Buffer, Length);
 return HAL_I2C_Mem_Read(&hi2c1, Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT, Buffer, Length, DISCOVERY_I2C1_TIMEOUT_MAX);
}

/**
  * @brief  MFX writes multiple data.
  * @param  Addr: I2C address
  * @param  Reg: Register address
  * @param  Buffer: Pointer to data buffer
  * @param  Length: Length of the data
  * @retval None
  */
void MFX_IO_WriteMultiple(uint16_t Addr, uint8_t Reg, uint8_t *Buffer, uint16_t Length)
{
//  I2C1_WriteBuffer(Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT, Buffer, Length);
  HAL_I2C_Mem_Write(&hi2c1, Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT, Buffer, Length, DISCOVERY_I2C1_TIMEOUT_MAX);
}

/**
  * @brief  MFX delay
  * @param  Delay: Delay in ms
  * @retval None
  */
void MFX_IO_Delay(uint32_t Delay)
{
  HAL_Delay(Delay);
}

void MFX_IO_DeInit(void)
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  /* Enable wakeup gpio clock */
  MFX_WAKEUP_GPIO_CLK_ENABLE();

  /* MFX wakeup pin configuration */
  GPIO_InitStruct.Pin   = MFX_WAKEUP_PIN;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Pull  = GPIO_PULLDOWN;
  HAL_GPIO_Init(MFX_WAKEUP_GPIO_PORT, &GPIO_InitStruct);

  /* DeInit interrupt pin : disable IRQ before to avoid spurious interrupt */
  HAL_NVIC_DisableIRQ((IRQn_Type)(MFX_INT_EXTI_IRQn));
  MFX_INT_GPIO_CLK_ENABLE();
  HAL_GPIO_DeInit(MFX_INT_GPIO_PORT, MFX_INT_PIN);

  /* DeInit the I2C */
  HAL_I2C_DeInit(&hi2c1);
  HAL_I2C_MspDeInit(&hi2c1);
}

/**
  * @brief  This function reset the OSPI memory.
  * @param  hospi: OSPI handle
  * @retval None
  */
static uint8_t ResetMemory(OSPI_HandleTypeDef *hospi)
{
  OSPI_RegularCmdTypeDef sCommand;
  OSPI_AutoPollingTypeDef sConfig;

  /* Initialize the reset enable command */
  sCommand.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
  sCommand.FlashId               = HAL_OSPI_FLASH_ID_1;
  sCommand.Instruction           = RESET_ENABLE_CMD;
  sCommand.InstructionMode       = HAL_OSPI_INSTRUCTION_1_LINE;
  sCommand.InstructionSize       = HAL_OSPI_INSTRUCTION_8_BITS;
  sCommand.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  sCommand.AddressMode           = HAL_OSPI_ADDRESS_NONE;
  sCommand.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode              = HAL_OSPI_DATA_NONE;
  sCommand.DummyCycles           = 0;
  sCommand.DQSMode               = HAL_OSPI_DQS_DISABLE;
  sCommand.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* Send the command */
  if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return OSPI_NOR_ERROR;
  }

  /* Send the reset memory command */
  sCommand.Instruction = RESET_MEMORY_CMD;
  if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return OSPI_NOR_ERROR;
  }

  /* Configure automatic polling mode to wait the memory is ready */
  sCommand.Instruction  = READ_STATUS_REG_CMD;
  sCommand.DataMode     = HAL_OSPI_DATA_1_LINE;
  sCommand.NbData       = 1;
  sCommand.DataDtrMode  = HAL_OSPI_DATA_DTR_DISABLE;

  sConfig.Match         = 0;
  sConfig.Mask          = MX25LM51245G_SR_WIP;
  sConfig.MatchMode     = HAL_OSPI_MATCH_MODE_AND;
  sConfig.Interval      = 0x10;
  sConfig.AutomaticStop = HAL_OSPI_AUTOMATIC_STOP_ENABLE;

  if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return OSPI_NOR_ERROR;
  }

  if (HAL_OSPI_AutoPolling(hospi, &sConfig, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return OSPI_NOR_ERROR;
  }

  return OSPI_NOR_OK;
}

/**
  * @brief  This function enables/disables the octal mode of the memory.
  * @param  hospi: OSPI handle
  * @param  Operation: Octal operation mode enable/disable
  * @retval None
  */
static uint8_t OctalMode(OSPI_HandleTypeDef *hospi, uint8_t Operation)
{
  OSPI_RegularCmdTypeDef  sCommand;
  OSPI_AutoPollingTypeDef sConfig;
  uint8_t reg[2];

  sCommand.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  sCommand.FlashId            = HAL_OSPI_FLASH_ID_1;
  sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  sCommand.AddressSize        = HAL_OSPI_ADDRESS_32_BITS;
  sCommand.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_DISABLE;
  sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataDtrMode        = HAL_OSPI_DATA_DTR_DISABLE;
  sCommand.DQSMode            = HAL_OSPI_DQS_DISABLE;
  sCommand.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  sConfig.MatchMode     = HAL_OSPI_MATCH_MODE_AND;
  sConfig.Interval      = 0x10;
  sConfig.AutomaticStop = HAL_OSPI_AUTOMATIC_STOP_ENABLE;

  /* Activate/deactivate the Octal mode */
  if (Operation == OSPI_NOR_OCTAL_ENABLE)
  {
    /* Enable write operations */
    sCommand.Instruction     = WRITE_ENABLE_CMD;
    sCommand.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.AddressMode     = HAL_OSPI_ADDRESS_NONE;
    sCommand.DataMode        = HAL_OSPI_DATA_NONE;
    sCommand.DummyCycles     = 0;

    if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return OSPI_NOR_ERROR;
    }

    /* Configure automatic polling mode to wait for write enabling */
    sConfig.Match = MX25LM51245G_SR_WEL;
    sConfig.Mask  = MX25LM51245G_SR_WEL;

    sCommand.Instruction = READ_STATUS_REG_CMD;
    sCommand.DataMode    = HAL_OSPI_DATA_1_LINE;
    sCommand.NbData      = 1;

    if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return OSPI_NOR_ERROR;
    }

    if (HAL_OSPI_AutoPolling(hospi, &sConfig, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return OSPI_NOR_ERROR;
    }

    /* Write Configuration register 2 (with new dummy cycles) */
    sCommand.Instruction = WRITE_CFG_REG_2_CMD;
    sCommand.Address     = MX25LM51245G_CR2_REG3_ADDR;
    sCommand.AddressMode = HAL_OSPI_ADDRESS_1_LINE;
    reg[0] = DUMMY_CYCLES_CR_CFG;

    if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return OSPI_NOR_ERROR;
    }

    if (HAL_OSPI_Transmit(hospi, reg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return OSPI_NOR_ERROR;
    }

    /* Enable write operations */
    sCommand.Instruction = WRITE_ENABLE_CMD;
    sCommand.AddressMode = HAL_OSPI_ADDRESS_NONE;
    sCommand.DataMode    = HAL_OSPI_DATA_NONE;

    if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return OSPI_NOR_ERROR;
    }

    /* Configure automatic polling mode to wait for write enabling */
    sCommand.Instruction = READ_STATUS_REG_CMD;
    sCommand.DataMode    = HAL_OSPI_DATA_1_LINE;

    if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return OSPI_NOR_ERROR;
    }

    if (HAL_OSPI_AutoPolling(hospi, &sConfig, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return OSPI_NOR_ERROR;
    }

    /* Write Configuration register 2 (with Octal I/O SPI protocol) */
    sCommand.Instruction = WRITE_CFG_REG_2_CMD;
    sCommand.Address     = MX25LM51245G_CR2_REG1_ADDR;
    sCommand.AddressMode = HAL_OSPI_ADDRESS_1_LINE;
#if defined BSP_OSPI_NOR_DDR_MODE_DEACTIVATED
    reg[0] = MX25LM51245G_CR2_SOPI;
#else
    reg[0] = MX25LM51245G_CR2_DOPI;
#endif

    if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return OSPI_NOR_ERROR;
    }

    if (HAL_OSPI_Transmit(hospi, reg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return OSPI_NOR_ERROR;
    }

    /* Wait that the configuration is effective and check that memory is ready */
    HAL_Delay(MX25LM51245G_WRITE_REG_MAX_TIME);

    if (AutoPollingMemReady(hospi, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != OSPI_NOR_OK)
    {
      return OSPI_NOR_ERROR;
    }

    /* Check the configuration has been correctly done */
    sCommand.Instruction     = OCTAL_READ_CFG_REG_2_CMD;
    sCommand.InstructionMode = HAL_OSPI_INSTRUCTION_8_LINES;
    sCommand.InstructionSize = HAL_OSPI_INSTRUCTION_16_BITS;
    sCommand.AddressMode     = HAL_OSPI_ADDRESS_8_LINES;
    sCommand.DataMode        = HAL_OSPI_DATA_8_LINES;
    sCommand.DummyCycles     = DUMMY_CYCLES_READ_OCTAL;
    sCommand.NbData          = 2;
    reg[0] = 0;

#if !defined BSP_OSPI_NOR_DDR_MODE_DEACTIVATED
    sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_ENABLE;
    sCommand.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_ENABLE;
    sCommand.DataDtrMode        = HAL_OSPI_DATA_DTR_ENABLE;
    sCommand.DQSMode            = HAL_OSPI_DQS_ENABLE;
#endif

    if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return OSPI_NOR_ERROR;
    }

    if (HAL_OSPI_Receive(hospi, reg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return OSPI_NOR_ERROR;
    }

#if defined BSP_OSPI_NOR_DDR_MODE_DEACTIVATED
    if (reg[0] != MX25LM51245G_CR2_SOPI)
#else
    if (reg[0] != MX25LM51245G_CR2_DOPI)
#endif
    {
      return OSPI_NOR_ERROR;
    }
  }
  else
  {
    /* Enable write operations */
    if (WriteEnable(hospi) != OSPI_NOR_OK)
    {
      return OSPI_NOR_ERROR;
    }

    /* Write Configuration register 2 (with Octal I/O SPI protocol) */
    sCommand.Instruction     = OCTAL_WRITE_CFG_REG_2_CMD;
    sCommand.InstructionMode = HAL_OSPI_INSTRUCTION_8_LINES;
    sCommand.InstructionSize = HAL_OSPI_INSTRUCTION_16_BITS;
    sCommand.Address         = MX25LM51245G_CR2_REG1_ADDR;
    sCommand.AddressMode     = HAL_OSPI_ADDRESS_8_LINES;
    sCommand.DataMode        = HAL_OSPI_DATA_8_LINES;
    sCommand.NbData          = 2;
    sCommand.DummyCycles     = 0;
    reg[0] = 0;
    reg[1] = 0;

#if !defined BSP_OSPI_NOR_DDR_MODE_DEACTIVATED
    sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_ENABLE;
    sCommand.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_ENABLE;
    sCommand.DataDtrMode        = HAL_OSPI_DATA_DTR_ENABLE;
#endif

    if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return OSPI_NOR_ERROR;
    }

    if (HAL_OSPI_Transmit(hospi, reg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return OSPI_NOR_ERROR;
    }

    /* Wait that the configuration is effective and check that memory is ready */
    HAL_Delay(MX25LM51245G_WRITE_REG_MAX_TIME);

    sCommand.Instruction     = READ_STATUS_REG_CMD;
    sCommand.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.AddressMode     = HAL_OSPI_ADDRESS_NONE;
    sCommand.DataMode        = HAL_OSPI_DATA_1_LINE;
    sCommand.NbData          = 1;
    sCommand.DummyCycles     = 0;

#if !defined BSP_OSPI_NOR_DDR_MODE_DEACTIVATED
    sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_DISABLE;
    sCommand.DataDtrMode        = HAL_OSPI_DATA_DTR_DISABLE;
#endif

    sConfig.Match = 0;
    sConfig.Mask  = MX25LM51245G_SR_WIP;

    if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return OSPI_NOR_ERROR;
    }

    if (HAL_OSPI_AutoPolling(hospi, &sConfig, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return OSPI_NOR_ERROR;
    }

    /* Check the configuration has been correctly done */
    sCommand.Instruction = READ_CFG_REG_2_CMD;
    sCommand.Address     = MX25LM51245G_CR2_REG1_ADDR;
    sCommand.AddressMode = HAL_OSPI_ADDRESS_1_LINE;

    if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return OSPI_NOR_ERROR;
    }

    if (HAL_OSPI_Receive(hospi, reg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return OSPI_NOR_ERROR;
    }

    if (reg[0] != 0)
    {
      return OSPI_NOR_ERROR;
    }
  }

  return OSPI_NOR_OK;
}

/**
  * @brief  This function read the SR of the memory and wait the EOP.
  * @param  hospi: OSPI handle
  * @param  Timeout: Timeout for auto-polling
  * @retval None
  */
static uint8_t AutoPollingMemReady(OSPI_HandleTypeDef *hospi, uint32_t Timeout)
{
  OSPI_RegularCmdTypeDef  sCommand;
  OSPI_AutoPollingTypeDef sConfig;
  uint8_t reg[2];

  /* Configure automatic polling mode to wait for memory ready */
  sCommand.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
  sCommand.FlashId               = HAL_OSPI_FLASH_ID_1;
  sCommand.Instruction           = OCTAL_READ_STATUS_REG_CMD;
  sCommand.InstructionMode       = HAL_OSPI_INSTRUCTION_8_LINES;
  sCommand.InstructionSize       = HAL_OSPI_INSTRUCTION_16_BITS;
  sCommand.Address               = 0;
  sCommand.AddressMode           = HAL_OSPI_ADDRESS_8_LINES;
  sCommand.AddressSize           = HAL_OSPI_ADDRESS_32_BITS;
  sCommand.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode              = HAL_OSPI_DATA_8_LINES;
  sCommand.NbData                = 2;
  sCommand.DummyCycles           = DUMMY_CYCLES_READ_OCTAL;
  sCommand.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;

#if defined BSP_OSPI_NOR_DDR_MODE_DEACTIVATED
  sCommand.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  sCommand.AddressDtrMode        = HAL_OSPI_ADDRESS_DTR_DISABLE;
  sCommand.DataDtrMode           = HAL_OSPI_DATA_DTR_DISABLE;
  sCommand.DQSMode               = HAL_OSPI_DQS_DISABLE;
#else
  sCommand.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_ENABLE;
  sCommand.AddressDtrMode        = HAL_OSPI_ADDRESS_DTR_ENABLE;
  sCommand.DataDtrMode           = HAL_OSPI_DATA_DTR_ENABLE;
  sCommand.DQSMode               = HAL_OSPI_DQS_ENABLE;
#endif

  sConfig.Match           = 0;
  sConfig.Mask            = MX25LM51245G_SR_WIP;

  do
  {
    HAL_Delay(1); /* sConfig.Interval(0x10) / Clock(55 MHz) = 0.29 ms */

    if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return OSPI_NOR_ERROR;
    }

    if (HAL_OSPI_Receive(hospi, reg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return OSPI_NOR_ERROR;
    }
  }while((reg[0] & sConfig.Mask ) != sConfig.Match);

  return OSPI_NOR_OK;
}

/**
  * @brief  This function send a Write Enable and wait it is effective.
  * @param  hospi: OSPI handle
  * @retval None
  */
static uint8_t WriteEnable(OSPI_HandleTypeDef *hospi)
{
  OSPI_RegularCmdTypeDef  sCommand;
  OSPI_AutoPollingTypeDef sConfig;
  uint8_t reg[2];

  /* Enable write operations */
  sCommand.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
  sCommand.FlashId               = HAL_OSPI_FLASH_ID_1;
  sCommand.Instruction           = OCTAL_WRITE_ENABLE_CMD;
  sCommand.InstructionMode       = HAL_OSPI_INSTRUCTION_8_LINES;
  sCommand.InstructionSize       = HAL_OSPI_INSTRUCTION_16_BITS;
  sCommand.AddressMode           = HAL_OSPI_ADDRESS_NONE;
  sCommand.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode              = HAL_OSPI_DATA_NONE;
  sCommand.DummyCycles           = 0;
  sCommand.DQSMode               = HAL_OSPI_DQS_DISABLE;
  sCommand.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;

#if defined BSP_OSPI_NOR_DDR_MODE_DEACTIVATED
  sCommand.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
#else
  sCommand.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_ENABLE;
#endif

  if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return OSPI_NOR_ERROR;
  }

  /* Configure automatic polling mode to wait for write enabling */
  sConfig.Match           = MX25LM51245G_SR_WEL;
  sConfig.Mask            = MX25LM51245G_SR_WEL;

  sCommand.Instruction    = OCTAL_READ_STATUS_REG_CMD;
  sCommand.Address        = 0x0;
  sCommand.AddressMode    = HAL_OSPI_ADDRESS_8_LINES;
  sCommand.AddressSize    = HAL_OSPI_ADDRESS_32_BITS;
  sCommand.DataMode       = HAL_OSPI_DATA_8_LINES;
  sCommand.NbData         = 2;

#if defined BSP_OSPI_NOR_DDR_MODE_DEACTIVATED
  sCommand.AddressDtrMode = HAL_OSPI_ADDRESS_DTR_DISABLE;
  sCommand.DataDtrMode    = HAL_OSPI_DATA_DTR_DISABLE;
  sCommand.DQSMode        = HAL_OSPI_DQS_DISABLE;
  sCommand.DummyCycles    = DUMMY_CYCLES_READ_OCTAL;
#else
  sCommand.AddressDtrMode = HAL_OSPI_ADDRESS_DTR_ENABLE;
  sCommand.DataDtrMode    = HAL_OSPI_DATA_DTR_ENABLE;
  sCommand.DQSMode        = HAL_OSPI_DQS_ENABLE;
  sCommand.DummyCycles    = 4;
#endif

  do
  {
    HAL_Delay(1); /* sConfig.Interval(0x10) / Clock(55 MHz) = 0.29 ms */

    if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return OSPI_NOR_ERROR;
    }

    if (HAL_OSPI_Receive(hospi, reg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return OSPI_NOR_ERROR;
    }
  }while((reg[0] & sConfig.Mask ) != sConfig.Match);

  return OSPI_NOR_OK;
}

/* Handle interrupt from MFX */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
//  osThreadResume(mfxServiceTaskHandle); // Ensure the thread is running in case All tasks were suspended
  osSemaphoreRelease(mfxBinarySemHandle);
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(100);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_TouchGFX_Task */
/**
* @brief Function implementing the guiTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_TouchGFX_Task */
__weak void TouchGFX_Task(void *argument)
{
  /* USER CODE BEGIN TouchGFX_Task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END TouchGFX_Task */
}

/* USER CODE BEGIN Header_StartMfxServiceTask */
/**
* @brief Function implementing the mfxServiceTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartMfxServiceTask */
void StartMfxServiceTask(void *argument)
{
  /* USER CODE BEGIN StartMfxServiceTask */

  /*
   * This task handles interrupts from MFX GPIO expander
   * Only Touch Controller Interrupt is handled
   */

  uint32_t statusGpio;

  /* Infinite loop */
  for(;;)
  {
    /* Try to acquire mutex, wait for until it's released by EXTI IRQ Callback */
    osSemaphoreAcquire(mfxBinarySemHandle, osWaitForever);

    /* Disable MFX interrupt to manage current one */
    HAL_NVIC_DisableIRQ(MFX_INT_EXTI_IRQn);

    uint32_t irqPending = MFX_IO_Read(IDD_I2C_ADDRESS, MFXSTM32L152_REG_ADR_IRQ_PENDING);

    /* GPIO IT from MFX */
    if(irqPending & 0x01 /* MFX_IRQ_PENDING_GPIO */)
    {
      statusGpio = BSP_IO_ITGetStatus(RIGHT_JOY_PIN | LEFT_JOY_PIN | UP_JOY_PIN | DOWN_JOY_PIN | SD_DETECT_PIN | TS_INT_PIN);

      /* Clear IO Expander IT */
      BSP_IO_ITClear(statusGpio);
    }

    irqPending &= ~ 0x01 /* MFX_IRQ_PENDING_GPIO */;
    if (irqPending)
    {
      MFX_IO_Write(IDD_I2C_ADDRESS, MFXSTM32L152_REG_ADR_IRQ_ACK, irqPending);
    }

    /* Re-enable MFX interrupt */
    HAL_NVIC_EnableIRQ(MFX_INT_EXTI_IRQn);

    if(statusGpio & TS_INT_PIN)
    {
      // Signal touch sampling task
      osSemaphoreRelease(touchBinarySemHandle);
    }
  }
  /* USER CODE END StartMfxServiceTask */
}

 /**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __asm("bkpt #0");
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
