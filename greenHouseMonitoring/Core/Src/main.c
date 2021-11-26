/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32l452xx.h"
#include "stdio.h"
#include "string.h"
#include "VL53L0X.h"
#include "I2CDev.h"
#include "mk_dht11.h"
#include "hx711.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

// sensor meresi adatok strukturaja
typedef struct
{
  uint16_t laser_sensor_distance;
  uint16_t ultra_sonic_sensor_distance;
  uint8_t temperature;
  uint8_t humidity;
  unsigned long weight;
  uint16_t moisture;
} SENSOR_VALUES_TYPE;


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LOAD_CELL_SCALE 379
#define DEVICE_ID 1
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;

RTC_HandleTypeDef hrtc;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim6;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
// sensor struct peldany
SENSOR_VALUES_TYPE sensor_values;

// uj sor a kiiratas fuggvenyhez
char newLine [] = "\n\r";

// realtime clock datum es ido
RTC_DateTypeDef rtc_date;
RTC_TimeTypeDef rtc_time;

// meresi idokoz percben
uint8_t measurement_time = 10;

// laser distance sensor struct peldany
VL53L0X sensor1;

// ultra sonic sensor segedvaltozok
uint32_t ultra_sonic_rising = 0;
uint32_t ultra_sonic_falling = 0;
uint32_t ultrasonic_difference = 0;
uint8_t ultra_sonic_frist_captured = 0;

// DHT11-MK sensor struct peldany
dht11_t dht_sensor;

// HX711 loadcell struct peldany
HX711 loadcell_sensor;

// UART-on erkezett adatok tarolasa
char uart_buffer[10] = "";

// azonnali meres kezdemenyezese
uint8_t rapidMeasure;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_RTC_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM6_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
 /**
  * Uzenet kiirasa consolera UART2 keresztul
  *
  * @param char message[]
  */
  void PrintMessage(char message[])
  {
	  HAL_UART_Transmit(&huart2,message, strlen(message), HAL_MAX_DELAY);
  }

  /**
   * Szam kiirasa consolera UART2 keresztul
   *
   * @param uint16_t number
   */
  void PrintNumber(uint16_t number)
    {
	  char converted_number[6];
	  sprintf(converted_number, "%d", number);
  	  PrintMessage(converted_number);
    }

  /**
   * Uj sor kiirasa consolera UART2 keresztul
   */
  void PrintNewLine()
  {
	  HAL_UART_Transmit(&huart2, (uint8_t*)newLine, sizeof(newLine), HAL_MAX_DELAY);
  }


  /**
   * RTC ido lekerdezese
   */
  void getTime()
  {
	  HAL_RTC_GetTime(&hrtc, &rtc_time, RTC_FORMAT_BIN);
	  HAL_RTC_GetDate(&hrtc, &rtc_date, RTC_FORMAT_BIN);
  }

  /**
   * Laser distance sensor inicializalasa
   */
  void initLaserDistanceSensor()
  {
	  setup_VL53L0X(&sensor1);
	  HAL_GPIO_WritePin(LASER_DISTANCE_XSHUT_GPIO_Port, LASER_DISTANCE_XSHUT_Pin, GPIO_PIN_RESET);
	  HAL_Delay(100);

	  HAL_GPIO_WritePin(LASER_DISTANCE_XSHUT_GPIO_Port, LASER_DISTANCE_XSHUT_Pin, GPIO_PIN_SET);
	  HAL_Delay(100);
	  if(!init(&sensor1,true))
	  {
			  PrintMessage("Laser Distance sensor inicializalas sikertelen");
			  PrintNewLine();
	  }
	  else
	  {
		  PrintMessage("Laser Distance sensor inicializalas sikeres");
		  PrintNewLine();
	  }
	  setTimeout(&sensor1,500);

	  setVcselPulsePeriod(&sensor1,VcselPeriodPreRange, 18);
	  setVcselPulsePeriod(&sensor1,VcselPeriodFinalRange, 14);
	  setMeasurementTimingBudget(&sensor1,200000);
  }

  /**
   * Laser distance sensor ertek beolvasas
   */
  void readLaserDistanceValue()
  {
	  sensor_values.laser_sensor_distance = readRangeSingleMillimeters(&sensor1);
  }

  /**
   * Ultrasonic distance sensor ertek beolvasas
   */
  void ultraSonicDisctanceRead(void) {
  	HAL_GPIO_WritePin(ULTRA_SONIC_TRG_GPIO_Port, ULTRA_SONIC_TRG_Pin, GPIO_PIN_SET);
  	HAL_Delay(100);
  	HAL_GPIO_WritePin(ULTRA_SONIC_TRG_GPIO_Port, ULTRA_SONIC_TRG_Pin, GPIO_PIN_RESET);
  	__HAL_TIM_ENABLE_IT(&htim1, TIM_IT_CC2);
  }

  /**
   * Homerseklet es paratartalom ertek beolvasas
   */
  void measureTemperatureAndHumidity()
  {
	  readDHT11(&dht_sensor);
	  sensor_values.temperature = dht_sensor.temperature;
	  sensor_values.humidity = dht_sensor.humidty;
  }

  /**
   * Suly cella inicializalas
   */
  void loadCellInit()
  {
	  loadcell_sensor.gpioData = LOAD_CELL_DT_GPIO_Port;
	  loadcell_sensor.gpioSck = LOAD_CELL_CLK_GPIO_Port;
	  loadcell_sensor.pinData = LOAD_CELL_DT_Pin;
	  loadcell_sensor.pinSck = LOAD_CELL_CLK_Pin;
	  loadcell_sensor.gain = 128;
	  loadcell_sensor.offset = 0;

	  HX711_Init(loadcell_sensor);
	  HAL_Delay(2000);
	  loadcell_sensor =  HX711_Tare(loadcell_sensor, 1);

  }

  /**
   * Tomeg mero cella ertek beolvasas es skalazasa
   */
  void measureWeight()
  {
	  sensor_values.weight = ((HX711_Value(loadcell_sensor) - (unsigned long)loadcell_sensor.offset) / LOAD_CELL_SCALE);
  }

  /**
   * Fold nedvesseg ertek beolvasas
   */
  void measureSoilMoisture()
  {
	  HAL_ADC_Start(&hadc1);
	  sensor_values.moisture =  HAL_ADC_GetValue(&hadc1);
	  HAL_ADC_Stop(&hadc1);
  }

  /**
   * JSON adat kuldese az ESP32 mikrovezerlonek
   */
  void sendJSONData()
  {
	  char jsonData[1000]= "";
	  sprintf(jsonData,
			  "{\"deviceId\": %d, \"laserDistance\": %d, \"sonicDistance\": %d,"
			  " \"temperature\": %d, \"humidity\": %d, \"weight\": %d, \"moisture\": %d}",
			  DEVICE_ID,
			  sensor_values.laser_sensor_distance,
			  sensor_values.ultra_sonic_sensor_distance,
			  sensor_values.temperature,
			  sensor_values.humidity,
			  sensor_values.weight,
			  sensor_values.moisture
			  );
	  HAL_UART_Transmit(&huart1, jsonData, strlen(jsonData), HAL_MAX_DELAY);
  }

  /**
   * Az osszes sensor adat beolvasasa
   */
  void measureAllData() {
	  readLaserDistanceValue();
	  PrintMessage("laser sensor ertek: ");
	  PrintNumber(sensor_values.laser_sensor_distance);
	  PrintNewLine();
	  ultraSonicDisctanceRead();
	  PrintMessage("us sensor ertek: ");
	  PrintNumber(sensor_values.ultra_sonic_sensor_distance);
	  PrintNewLine();
	  measureTemperatureAndHumidity();
	  PrintMessage("temp sensor ertek: ");
	  PrintNumber(sensor_values.temperature);
	  PrintNewLine();
	  PrintMessage("hum sensor ertek: ");
	  PrintNumber(sensor_values.humidity);
	  PrintNewLine();
	  measureWeight();
	  PrintMessage("weight sensor ertek: ");
	  PrintNumber(sensor_values.weight);
	  PrintNewLine();
	  measureSoilMoisture();
	  PrintMessage("moisture sensor ertek: ");
	  PrintNumber(sensor_values.moisture);
	  PrintNewLine();
  }

  void I2C_Read_Byte(uint8_t addr,uint8_t *data)
  {
   while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY);
  if(HAL_I2C_Master_Transmit(&hi2c1,  SLAVE_ADDRESS<<1 , &addr,  1, 100)!=HAL_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }
  while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY);
  if(HAL_I2C_Master_Receive(&hi2c1, ((SLAVE_ADDRESS <<1) | 0x01), data, 1, 100)!=HAL_OK)
    {
       _Error_Handler(__FILE__, __LINE__);
    }
  }

  void I2C_Write_Byte(uint8_t addr, uint8_t data)
  {
    uint8_t buf[] = {addr, data};
    while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY);
    if(HAL_I2C_Master_Transmit(&hi2c1, SLAVE_ADDRESS<<1, buf, 2, 100)!=HAL_OK)
      {
        _Error_Handler(__FILE__, __LINE__);
      }
  }

  void readMulti(VL53L0X * lidar,uint8_t reg, uint8_t * dst, uint8_t count)
   {
       /* Send register address */
       if (HAL_I2C_Master_Transmit(&hi2c1, SLAVE_ADDRESS<<1, &reg, 1, 1000) != HAL_OK)
         {
           {
               _Error_Handler(__FILE__, __LINE__);
           }
         }

       /* Receive multiple byte */
       if (HAL_I2C_Master_Receive(&hi2c1, ((SLAVE_ADDRESS <<1) | 0x01), dst, count, 1000) != HAL_OK)
         {
            _Error_Handler(__FILE__, __LINE__);
         }
   }

   void writeMulti(VL53L0X * lidar,uint8_t reg, uint8_t* src, uint8_t count)
   {
            /* Try to transmit via I2C */
           if(HAL_I2C_Mem_Write(&hi2c1, SLAVE_ADDRESS<<1, reg, reg>0xFF ? I2C_MEMADD_SIZE_16BIT : I2C_MEMADD_SIZE_8BIT, src, count, 1000) != HAL_OK)
               {
                  _Error_Handler(__FILE__, __LINE__);
               }

   }
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
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_RTC_Init();
  MX_TIM1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */

  // sensor inicializalasok
  initLaserDistanceSensor();
  HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_2);
  init_dht11(&dht_sensor, &htim6, DHT11_GPIO_Port, DHT11_Pin);
  loadCellInit();
  measureAllData();
  HAL_UART_Receive_IT(&huart1, uart_buffer, 2);
  HAL_UART_Transmit(&huart1, "S", 1, HAL_MAX_DELAY);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  if((rtc_time.Seconds == 0 && (rtc_time.Minutes % measurement_time) == 0))
	  {
		  measureAllData();
		  sendJSONData();
	  }

	  if(rapidMeasure == 1)
	  {
		measureAllData();
		sendJSONData();
		rapidMeasure = 0;
	  }

	  getTime();
	  HAL_Delay(1000);
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USART1
                              |RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_I2C1
                              |RCC_PERIPHCLK_ADC;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_PLLSAI1;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  PeriphClkInit.PLLSAI1.PLLSAI1Source = RCC_PLLSOURCE_HSI;
  PeriphClkInit.PLLSAI1.PLLSAI1M = 1;
  PeriphClkInit.PLLSAI1.PLLSAI1N = 8;
  PeriphClkInit.PLLSAI1.PLLSAI1P = RCC_PLLP_DIV7;
  PeriphClkInit.PLLSAI1.PLLSAI1Q = RCC_PLLQ_DIV2;
  PeriphClkInit.PLLSAI1.PLLSAI1R = RCC_PLLR_DIV2;
  PeriphClkInit.PLLSAI1.PLLSAI1ClockOut = RCC_PLLSAI1_ADC1CLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_12CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

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
  hi2c1.Init.Timing = 0x00702991;
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

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 80-1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_IC_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim1, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 80-1;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 65535;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, DHT11_Pin|LOAD_CELL_DT_Pin|LOAD_CELL_CLK_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(ULTRA_SONIC_TRG_GPIO_Port, ULTRA_SONIC_TRG_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LASER_DISTANCE_XSHUT_GPIO_Port, LASER_DISTANCE_XSHUT_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : DHT11_Pin LOAD_CELL_DT_Pin LOAD_CELL_CLK_Pin */
  GPIO_InitStruct.Pin = DHT11_Pin|LOAD_CELL_DT_Pin|LOAD_CELL_CLK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : ULTRA_SONIC_TRG_Pin */
  GPIO_InitStruct.Pin = ULTRA_SONIC_TRG_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(ULTRA_SONIC_TRG_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LASER_DISTANCE_XSHUT_Pin */
  GPIO_InitStruct.Pin = LASER_DISTANCE_XSHUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LASER_DISTANCE_XSHUT_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void _Error_Handler(char *file, int line)
{
  /* USER CODE END Error_Handler_Debug */
}

// Timer az ultrahang tavolsag szenzor ertekenek olvasasara
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {

	if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) {
		if (ultra_sonic_frist_captured == 0) {
			ultra_sonic_rising = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
			ultra_sonic_frist_captured = 1;
			__HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_2,
					TIM_INPUTCHANNELPOLARITY_FALLING);
		} else if (ultra_sonic_frist_captured) {
			ultra_sonic_falling = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
			__HAL_TIM_SET_COUNTER(htim, 0);
			if (ultra_sonic_falling > ultra_sonic_rising) {
				ultrasonic_difference = ultra_sonic_falling - ultra_sonic_rising;
			} else if (ultra_sonic_rising > ultra_sonic_falling) {
				ultrasonic_difference = (65535 - ultra_sonic_rising) + ultra_sonic_falling;
			}
			sensor_values.ultra_sonic_sensor_distance = ultrasonic_difference * 0.34 / 2;
			ultra_sonic_frist_captured = 0;
			__HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_2,
					TIM_INPUTCHANNELPOLARITY_RISING);
			__HAL_TIM_DISABLE_IT(htim, TIM_IT_CC2);

		}
	}
}

// Interrupt callback az UART-on erkezo adatokra
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    HAL_UART_Receive_IT(&huart1, uart_buffer, 2);

	if (uart_buffer[0] == '0')
	{
		rapidMeasure = 1;
	}
	else
	{
		sscanf (uart_buffer,"%d",&measurement_time);
	}
}
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
